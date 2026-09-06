// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionExecutorSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GameplayCueManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Magicult/Character/MagicultCharacterBase.h"
#include "Magicult/AbilitySystem/Abilities/KnockbackGameplayAbility.h"
#include "Components/PrimitiveComponent.h"
#include "Magicult/Actor/MagicultProjectile.h"
#include "Magicult/Attack/AttackInstance.h"
#include "Magicult/Subsystem/CombatResolveSubsystem.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/Subsystem/CombatSubsystem.h"
#include "Magicult/Game/MagicultGameInstance.h"
#include "Magicult/Data/ImmunityPolicy.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

TArray<FSpellTriggerEventData> UActionExecutorSubsystem::ExecuteRequest(const TArray<FSpellActionExecutionRequest>& Request, int32& OutActionsExecuted)
{
	OutActionsExecuted = 0;

	TArray<FSpellTriggerEventData> ResultTriggerEventData;

	TQueue<FSpellActionExecutionRequest> PendingExecutionQueue;

	for (const FSpellActionExecutionRequest& ExecutionRequest : Request)
	{
		PendingExecutionQueue.Enqueue(ExecutionRequest);
	}

	static constexpr int32 MaxActionChainDepth = 8;

	while (!PendingExecutionQueue.IsEmpty())
	{
		FSpellActionExecutionRequest CurrentExecutionRequest;
		PendingExecutionQueue.Dequeue(CurrentExecutionRequest);

		// 깊이 초과 요청만 개별 스킵 - 같은 배치의 독립 요청들은 영향 없음.
		// 순환(예: 원소 반응이 또 원소 반응을 생성)은 깊이로만 자라므로 여기서 잡힌다.
		if (CurrentExecutionRequest.ChainDepth > MaxActionChainDepth)
		{
			UE_LOG(LogTemp, Warning, TEXT("ActionExecutor: MaxActionChainDepth(%d) exceeded. ActionType=%d"),
				MaxActionChainDepth, static_cast<int32>(CurrentExecutionRequest.Action.ActionType));
			continue;
		}

		const FSpellActionSpec& CurrentAction = CurrentExecutionRequest.Action;
		const FSpellContext& CurrentContext = CurrentExecutionRequest.Context;

		FSpellActionExecutionResult Result;
		{
			// 액션 1건 = Insights 타임라인의 개별 이벤트. Timers 패널에서 이 스코프 이름으로
			// 발생 횟수(Count)·총시간(Total)·평균시간(Avg)이 자동 집계된다.
			TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ExecuteSingleAction);
			Result = ExecuteSingleAction(CurrentAction, CurrentContext);
		}

		if (Result.bDidWork)
		{
			++OutActionsExecuted;
		}

		// 파생 요청에 부모 깊이 +1 전파
		for (FSpellActionExecutionRequest GeneratedRequest : Result.GeneratedActionRequests)
		{
			GeneratedRequest.ChainDepth = CurrentExecutionRequest.ChainDepth + 1;
			PendingExecutionQueue.Enqueue(GeneratedRequest);
		}

		if (Result.GeneratedTriggerEvents.Num() > 0)
		{
			ResultTriggerEventData.Append(Result.GeneratedTriggerEvents);
		}

		// [POST] 성공 적용된 액션에 대해 InterruptPolicy 기반 인터럽트 요청 생성
		if (Result.bApplied && Result.ResultTag.IsValid())
		{
			if (UCombatResolveSubsystem* CRS = GetWorld()->GetSubsystem<UCombatResolveSubsystem>())
			{
				TArray<FSpellActionExecutionRequest> InterruptRequests = CRS->ResolveInterrupt(Result.ResultTag, CurrentContext);
				for (FSpellActionExecutionRequest Req : InterruptRequests)
				{
					// 인터럽트도 파생 요청이므로 깊이 +1
					Req.ChainDepth = CurrentExecutionRequest.ChainDepth + 1;
					PendingExecutionQueue.Enqueue(Req);
				}
			}
		}
	}

	return ResultTriggerEventData;
}

bool UActionExecutorSubsystem::CheckImmunity(const FGameplayTag& Tag, const FSpellContext& Context) const
{
	const UAbilitySystemComponent* TargetASC = Context.TargetAbilitySystemComponent.Get();
	if (!TargetASC) return false;

	const UMagicultGameInstance* GI = GetWorld()->GetGameInstance<UMagicultGameInstance>();
	if (!GI || !GI->ImmunityPolicy)
	{
		UE_LOG(LogTemp, Error, TEXT("[ImmunityPolicy] GameInstance에 ImmunityPolicy가 설정되지 않았습니다. 면역 없음으로 처리합니다."));
		return false;
	}

	FGameplayTag ImmunityTag;
	if (!GI->ImmunityPolicy->FindImmunityTag(Tag, ImmunityTag)) return false;

	return TargetASC->HasMatchingGameplayTag(ImmunityTag);
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteSingleAction(const FSpellActionSpec& Action, const FSpellContext& Context)
{
	// 액션 권한 분리: EmitNiagara는 복제 수단이 없어 코스메틱으로 서버/클라 모두 실행하되 전용
	// 서버에선 렌더러가 없으므로 스킵한다. PlayCue는 ExecuteGameplayCue 자체가 서버 실행 시
	// ASC 멀티캐스트로 복제되므로(엔진 기본값) 권위 액션(Damage·GE·Knockback 등)과 동일하게
	// 서버 전용이다. SpawnAttackInstance는 게임 상태를 바꾸지 않는 Trigger 발생기라 모든
	// 머신에서 항상 실행하며, 여기서 생성되는 후속 Trigger(예: ApplyDamage)가 실행될 때
	// 이 필터를 다시 통과해 권위를 따로 거른다.
	const bool bIsCosmetic = Action.ActionType == ESpellActionType::EmitNiagara;

	const bool bAlwaysExecutes = Action.ActionType == ESpellActionType::SpawnAttackInstance;

	if (!bAlwaysExecutes &&
	    ((bIsCosmetic && GetWorld()->IsNetMode(NM_DedicatedServer)) ||
	     (!bIsCosmetic && GetWorld()->IsNetMode(NM_Client))))
	{
		// 이 머신에 실행 권한이 없는 액션 - 아무 일도 안 했으므로 프레임 예산 집계에서 제외.
		FSpellActionExecutionResult SkippedResult;
		SkippedResult.bDidWork = false;
		return SkippedResult;
	}

	switch (Action.ActionType)
	{
	case ESpellActionType::ConsumePierce:
		return ExecuteConsumePierce(Action.ConsumePierceSpec, Context);
	case ESpellActionType::ApplyDamage:
		return ExecuteApplyDamage(Context);
	case ESpellActionType::ApplyElement:
		return ExecuteResolveElementInteraction(Action.ApplyElementSpec, Context);
	case ESpellActionType::ApplyDebuff:
		return ExecuteApplyDebuff(Action.DebuffSpec, Context);
	case ESpellActionType::SpawnActor:
		// TODO : SpawnActorSpec에서 TriggerAction 설정하도록
		return ExecuteSpawnActor(Action.SpawnActorSpec, Context);
	case ESpellActionType::ApplyGameplayEffect:
		return ExecuteGameplayEffect(Action.ApplyGESpec, Context);
	case ESpellActionType::EmitNiagara:
		return ExecuteEmitNiagara(Action.EmitNiagaraSpec, Context);
	case ESpellActionType::PlayCue:
		return ExecutePlayCue(Action.PlayCueSpec, Context);
	case ESpellActionType::Knockback:
		return ExecuteKnockback(Action.KnockbackSpec, Context);
	case ESpellActionType::DestroyActor:
		return ExecuteDestroyActor(Context);
	case ESpellActionType::Interrupt:
		return ExecuteInterrupt(Action.InterruptSpec, Context);
	case ESpellActionType::SpawnAttackInstance:
		return ExecuteSpawnAttackInstance(Action.SpawnAttackInstanceSpec, Context);
	default:
		break;
	}

	return FSpellActionExecutionResult();
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteConsumePierce(const FSpellConsumePierceActionSpec& ConsumePierceSpec, const FSpellContext& Context)
{
	FSpellActionExecutionResult Result;

	AMagicultProjectile* Projectile = Cast<AMagicultProjectile>(Context.DamageCauser.Get());
	if (!Projectile)
	{
		return Result;
	}

	FSpellRuntimeState& RuntimeState = Projectile->ResolvedProjectileResult.RuntimeState;
	RuntimeState.RemainingPierceCount = FMath::Max(0, RuntimeState.RemainingPierceCount - ConsumePierceSpec.ConsumePierce);

	if (RuntimeState.RemainingPierceCount <= 0)
	{
		FSpellTriggerEventData TriggerData;
		TriggerData.TriggerTag = ConsumePierceSpec.ActivateTrigger.TriggerTag;
		TriggerData.TriggerContextTags = ConsumePierceSpec.ActivateTrigger.TriggerContextTags;
		TriggerData.SpellContext = Context;

		Result.GeneratedTriggerEvents.Add(TriggerData);
	}

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteApplyDamage(const FSpellContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ExecuteApplyDamage);
	FSpellActionExecutionResult Result;

	if (CheckImmunity(FMagicultGameplayTags::Get().Action_Damage, Context))
		return FSpellActionExecutionResult(false);

	UAbilitySystemComponent* SourceASC = Context.SourceAbilitySystemComponent.Get();
	UAbilitySystemComponent* TargetASC = Context.TargetAbilitySystemComponent.Get();
	if (!TargetASC)
	{
		return FSpellActionExecutionResult(false);
	}

	if (const IAttackContextProvider* Provider = Cast<IAttackContextProvider>(Context.DamageCauser.Get()))
	{
		if (Provider->GetAttackSpec().bTargetEnemiesOnly)
		{
			AActor* SourceActor = Context.SourceActor.Get();
			AActor* TargetActor = Context.TargetActor.Get();
			if (SourceActor && TargetActor && !UMagicultAbilitySystemLibrary::IsNotFriend(SourceActor, TargetActor))
				return FSpellActionExecutionResult(false);
		}
	}

	const bool bAppliedDamage = UMagicultAbilitySystemLibrary::ApplyDefaultDamageGameplayEffect(Context, SourceASC, TargetASC);

	if (!bAppliedDamage)
	{
		return Result;
	}

	FSpellTriggerEventData TriggerData;
	TriggerData.TriggerTag = FMagicultGameplayTags::Get().Trigger_OnAttack;
	TriggerData.SpellContext = Context;

	TriggerData.TriggerContextTags.AddTag(FMagicultGameplayTags::Get().AttackType_Direct);

	// 필요하면 후속 TriggerTags 설계 가능
	Result.GeneratedTriggerEvents.Add(TriggerData);

	if (const IAttackContextProvider* AttackContextProvider = Cast<IAttackContextProvider>(Context.DamageCauser.Get()))
	{
		if (!AttackContextProvider->GetAttackSpec().ElementTag.IsValid())
		{
			Result.ResultTag = FMagicultGameplayTags::Get().Action_Damage;
			return Result;
		}

		FSpellActionSpec SpellAction;
		SpellAction.ActionType = ESpellActionType::ApplyElement;
		SpellAction.ApplyElementSpec.IncomingElement = AttackContextProvider->GetAttackSpec().ElementTag;

		Result.GeneratedActionRequests.Emplace(SpellAction, Context);
	}

	Result.ResultTag = FMagicultGameplayTags::Get().Action_Damage;
	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteResolveElementInteraction(const FSpellApplyElementActionSpec& ApplyElementSpec, const FSpellContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ExecuteResolveElement);
	FSpellActionExecutionResult Result;

	UCombatResolveSubsystem* CombatResolveSubsystem = GetWorld()->GetSubsystem<UCombatResolveSubsystem>();
	if (!CombatResolveSubsystem)
	{
		return Result;
	}

	FElementInteractionResolveResult ElementInteractionResolveResult = CombatResolveSubsystem->ResolveIncomingElement(Context, ApplyElementSpec.IncomingElement);

	Result.GeneratedActionRequests.Append(ElementInteractionResolveResult.InteractionActionRequests);
	Result.GeneratedActionRequests.Append(ElementInteractionResolveResult.DebuffActionRequests);

	UAbilitySystemComponent* TargetASC = Context.TargetAbilitySystemComponent.Get();
	if (!TargetASC)
	{
		return Result;
	}

	for (const FGameplayTag& RemoveTag : ElementInteractionResolveResult.RemoveTags)
	{
		if (!RemoveTag.IsValid())
		{
			continue;
		}

		FGameplayTagContainer TagsToRemove;
		TagsToRemove.AddTag(RemoveTag);
		TargetASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
	}

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteApplyDebuff(const FSpellDebuffActionSpec& DebuffSpec, const FSpellContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ExecuteApplyDebuff);
	FSpellActionExecutionResult Result;

	if (CheckImmunity(FMagicultGameplayTags::Get().Action_Debuff, Context))
		return FSpellActionExecutionResult(false);

	UAbilitySystemComponent* SourceASC = Context.SourceAbilitySystemComponent.Get();

	AActor* TargetActor = ResolveActorByPolicy(DebuffSpec.TargetPolicy, Context);
	UAbilitySystemComponent* TargetASC = TargetActor
		? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor)
		: nullptr;
	if (!SourceASC || !TargetASC)
	{
		return FSpellActionExecutionResult(false);
	}

	// 사망한 대상에게는 디버프 적용 안 함
	if (TargetActor && TargetActor->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(TargetActor))
	{
		return FSpellActionExecutionResult(false);
	}

	// 개별 디버프 타입 면역 (예: Debuff.Freeze)
	if (DebuffSpec.DebuffTag.IsValid() && CheckImmunity(DebuffSpec.DebuffTag, Context))
	{
		return FSpellActionExecutionResult(false);
	}

	// 기존 동일 디버프 제거 후 갱신
	if (DebuffSpec.DebuffTag.IsValid())
	{
		FGameplayTagContainer TagsToRemove;
		TagsToRemove.AddTag(DebuffSpec.DebuffTag);
		TargetASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
	}

	for (const FDebuffEffectEntry& EffectEntry : DebuffSpec.EffectSpecs)
	{
		if (EffectEntry.Tag.IsValid() && CheckImmunity(EffectEntry.Tag, Context)) continue;

		if (!EffectEntry.EffectClass)
		{
			continue;
		}

		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(Context.DamageCauser.Get() ? static_cast<UObject*>(Context.DamageCauser.Get()) : static_cast<UObject*>(Context.SourceActor.Get()));

		if (Context.bHasHitResult)
		{
			EffectContext.AddHitResult(Context.HitResult);
		}

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectEntry.EffectClass, 1.f, EffectContext);

		if (!SpecHandle.IsValid())
		{
			continue;
		}

		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
		if (!Spec)
		{
			continue;
		}

		if (DebuffSpec.DebuffTag.IsValid())
		{
			Spec->DynamicGrantedTags.AddTag(DebuffSpec.DebuffTag);
		}

		switch (EffectEntry.Type)
		{
		case EDebuffEffectType::Slow:
			Spec->SetSetByCallerMagnitude(
				FMagicultGameplayTags::Get().Data_Modifier,
				EffectEntry.SlowData.MoveSpeedMultiplier);

			Spec->SetSetByCallerMagnitude(
				FMagicultGameplayTags::Get().Data_Duration,
				EffectEntry.SlowData.Duration);
			break;

		case EDebuffEffectType::PeriodicDamage:
			Spec->SetSetByCallerMagnitude(
				FMagicultGameplayTags::Get().Data_Damage,
				EffectEntry.DamageData.DamagePerTick);

			Spec->Period = EffectEntry.DamageData.TickInterval;

			Spec->SetSetByCallerMagnitude(
				FMagicultGameplayTags::Get().Data_Duration,
				EffectEntry.DamageData.Duration);
			break;
		default:
			break;
		}

		TargetASC->ApplyGameplayEffectSpecToSelf(*Spec);
	}

	Result.ResultTag = DebuffSpec.DebuffTag;
	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteSpawnActor(const FSpellSpawnActorActionSpec& SpawnActorSpec, const FSpellContext& Context)
{
	FSpellActionExecutionResult Result;

	UWorld* World = GetWorld();
	UClass* ActorClass = SpawnActorSpec.ActorClass;
	if (!World || !ActorClass)
	{
		return Result;
	}

	FTransform BaseTransform = FTransform::Identity;

	if (SpawnActorSpec.bAttachToHitTarget && Context.TargetActor.IsValid())
	{
		BaseTransform = Context.TargetActor->GetActorTransform();
	}
	else if (Context.bHasHitResult)
	{
		BaseTransform = FTransform(Context.HitResult.ImpactNormal.Rotation(), Context.HitResult.ImpactPoint);
	}
	else if (AActor* DamageCauserActor = Cast<AActor>(Context.DamageCauser.Get()))
	{
		BaseTransform = DamageCauserActor->GetActorTransform();
	}
	else if (Context.SourceActor.IsValid())
	{
		BaseTransform = Context.SourceActor->GetActorTransform();
	}

	const FTransform SpawnTransform = SpawnActorSpec.RelativeTransform * BaseTransform;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Context.SourceActor.Get();
	SpawnParams.Instigator = Cast<APawn>(Context.SourceActor.Get());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);

	// TODO
	/*if (SpawnedActor && SpawnActorSpec.bAttachToHitTarget && Context.TargetActor.IsValid())
	{
		SpawnedActor->AttachToActor(Context.TargetActor.Get(), FAttachmentTransformRules::KeepWorldTransform);
	}*/

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteSpawnAttackInstance(const FSpellSpawnAttackInstanceActionSpec& Spec, const FSpellContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ExecuteSpawnAttackInstance);
	FSpellActionExecutionResult Result;

	if (!Spec.AttackInstanceClass)
	{
		return Result;
	}

	UAttackInstance* NewCauser = NewObject<UAttackInstance>(this, Spec.AttackInstanceClass);
	if (!NewCauser)
	{
		return Result;
	}

	// origin·부모 스펙은 DamageCauser 교체 *전*의 원본 Context로 해석한다.
	// (DamageCauser 정책이면 부모 causer(예: 발사체)의 위치를 가져와야 하고,
	//  스펙도 부모 causer의 것을 물려받아야 하므로.)
	const FVector Origin = ResolveOriginByPolicy(Spec.OriginPolicy, Context, Spec.SocketTag);

	// 데미지 필드(BaseDamage/Element/Modifier/velocity)는 전부 DamageCauser.GetAttackSpec()에서
	// 나오므로(FillDamageEffectContext), 발사체 폭발이 발사체 데미지를 쓰려면 부모 스펙을 물려줘야 한다.
	FAttackSpec InheritedSpec;
	bool bOverrideSpec = false;
	if (Spec.bInheritSpecFromCauser)
	{
		if (const IAttackContextProvider* ParentProvider = Cast<IAttackContextProvider>(Context.DamageCauser.Get()))
		{
			InheritedSpec = ParentProvider->GetAttackSpec();
			bOverrideSpec = true;
		}
	}

	// 이 causer가 발행할 트리거의 DamageCauser는 "자기 자신"이어야 한다.
	// (ProcessTriggerQueue가 DamageCauser로 TriggerActionBindings를 조회하므로.)
	// GA가 MakeSpellContext(..., CurrentAttackInstance)로 하는 것과 동일 패턴.
	FSpellContext CauserContext = Context;                       // TriggerChainDepth 등 상속
	CauserContext.DamageCauser = NewCauser;                      // weak — 정체성 교체
	CauserContext.TargetActor = nullptr;                         // 대상은 수집 전이므로 미정
	CauserContext.TargetAbilitySystemComponent = nullptr;

	// bInheritSpecFromCauser=false면 bOverrideSpec=false → AttackInstance 자신의 기본 Spec 유지.
	NewCauser->Initialize(CauserContext, InheritedSpec, bOverrideSpec);
	NewCauser->SetAttackOrigin(Origin);                          // policy로 해석한 수집 중심 주입
	NewCauser->SetInstanceConfig(Spec.InstanceConfig);           // InitializeWithConfig()로 반경 등 세팅
	NewCauser->SetOwnerActor(Context.SourceActor.Get());
	NewCauser->Execute();

	// 즉시 실행: 생성 직후 활성화 태그를 스스로에게 전달해 동기적으로 수집·발행하게 한다.
	// NewCauser는 이 함수 리턴 후 아무도 강참조하지 않으므로, 여기서 발행한 트리거가
	// EnqueueInternal의 pin을 걸어야만(§Pin) 리턴 이후에도 살아남아 처리된다.
	NewCauser->HandleEventTag(FMagicultGameplayTags::Get().Notify_Attack_Commit);

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteGameplayEffect(const FSpellApplyGEActionSpec& ApplyGESpec, const FSpellContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ExecuteGameplayEffect);
	FSpellActionExecutionResult Result;

	UAbilitySystemComponent* SourceASC = Context.SourceAbilitySystemComponent.Get();
	UAbilitySystemComponent* TargetASC = Context.TargetAbilitySystemComponent.Get();
	if (!SourceASC || !TargetASC || !ApplyGESpec.GameplayEffectClass)
	{
		return Result;
	}

	if (!ApplyGESpec.TargetFilterTags.IsEmpty())
	{
		FGameplayTagContainer OwnedTags;
		TargetASC->GetOwnedGameplayTags(OwnedTags);

		if (!OwnedTags.HasAll(ApplyGESpec.TargetFilterTags))
		{
			return Result;
		}
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(Context.SourceActor.Get());

	if (Context.bHasHitResult)
	{
		EffectContext.AddHitResult(Context.HitResult);
	}

	FGameplayEffectSpecHandle SpecHandle =
		SourceASC->MakeOutgoingSpec(ApplyGESpec.GameplayEffectClass, 1.f, EffectContext);

	if (!SpecHandle.IsValid())
	{
		return Result;
	}

	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	return Result;
}

AActor* UActionExecutorSubsystem::ResolveActorByPolicy(ESpellActionTargetPolicy Policy, const FSpellContext& Context) const
{
	switch (Policy)
	{
	case ESpellActionTargetPolicy::Source:
	case ESpellActionTargetPolicy::SocketTransform:
		return Context.SourceActor.Get();

	case ESpellActionTargetPolicy::Target:
	case ESpellActionTargetPolicy::HitActor:
		return Context.TargetActor.Get();

	case ESpellActionTargetPolicy::DamageCauser:
		if (AActor* DamageCauserActor = Cast<AActor>(Context.DamageCauser.Get()))
		{
			return DamageCauserActor;
		}
		return Context.TargetActor.Get();

	default:
		return nullptr;
	}
}

FTransform UActionExecutorSubsystem::ResolveTransformByPolicy(ESpellActionTargetPolicy Policy, const FSpellContext& Context, FGameplayTag SocketTag, bool* bOutResolved) const
{
	if (bOutResolved) *bOutResolved = true;

	switch (Policy)
	{
	case ESpellActionTargetPolicy::HitLocation:
		if (Context.bHasHitResult)
		{
			return FTransform(Context.HitResult.ImpactPoint);
		}
		break;

	case ESpellActionTargetPolicy::SocketTransform:
		if (AActor* Actor = ResolveActorByPolicy(Policy, Context))
		{
			if (Actor->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()) && SocketTag.IsValid())
				return ICombatInterface::Execute_GetCombatSocketLocation(Actor, SocketTag);
			return Actor->GetActorTransform();
		}
		break;

	case ESpellActionTargetPolicy::DamageCauser:
		// Actor causer는 자기 트랜스폼, UObject causer(AttackInstance 등)는 주입된 origin.
		if (AActor* CauserActor = Cast<AActor>(Context.DamageCauser.Get()))
		{
			return CauserActor->GetActorTransform();
		}
		if (const IAttackContextProvider* Provider = Cast<IAttackContextProvider>(Context.DamageCauser.Get()))
		{
			return FTransform(Provider->GetAttackOrigin());
		}
		break;

	case ESpellActionTargetPolicy::Source:
	case ESpellActionTargetPolicy::Target:
	case ESpellActionTargetPolicy::HitActor:
		if (AActor* Actor = ResolveActorByPolicy(Policy, Context))
		{
			return Actor->GetActorTransform();
		}
		break;

	default:
		break;
	}

	if (bOutResolved) *bOutResolved = false;
	return FTransform::Identity;
}

FVector UActionExecutorSubsystem::ResolveOriginByPolicy(ESpellActionTargetPolicy Policy, const FSpellContext& Context, FGameplayTag SocketTag) const
{
	bool bResolved = false;
	const FTransform Resolved = ResolveTransformByPolicy(Policy, Context, SocketTag, &bResolved);
	if (bResolved)
	{
		return Resolved.GetLocation();
	}

	// fallback: origin이 조용히 (0,0,0)이 되어 월드 원점에서 터지는 사고를 막는다.
	if (const AActor* CauserActor = Cast<AActor>(Context.DamageCauser.Get()))
	{
		return CauserActor->GetActorLocation();
	}
	if (const AActor* SourceActor = Context.SourceActor.Get())
	{
		return SourceActor->GetActorLocation();
	}
	return FVector::ZeroVector;
}

// SpawnSystemAtLocation은 복제되지 않는다 - 이 액션을 실행한 머신에서만 보인다.
// 전원에게 보여야 하는 이펙트는 PlayCue(서버 실행 -> ASC 멀티캐스트)를 쓸 것.
FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteEmitNiagara(const FSpellEmitNiagaraActionSpec& EmitNiagaraSpec, const FSpellContext& Context)
{
	FSpellActionExecutionResult Result;

	if (!EmitNiagaraSpec.Niagara)
	{
		return Result;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return Result;
	}

	const FTransform BaseTransform = ResolveTransformByPolicy(EmitNiagaraSpec.TargetPolicy, Context, EmitNiagaraSpec.SocketTag);
	const FTransform SpawnTransform = BaseTransform * EmitNiagaraSpec.RelativeTransform;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		EmitNiagaraSpec.Niagara,
		SpawnTransform.GetLocation(),
		SpawnTransform.Rotator(),
		SpawnTransform.GetScale3D()
	);

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecutePlayCue(const FSpellPlayCueActionSpec& PlayCueSpec, const FSpellContext& Context)
{
	FSpellActionExecutionResult Result;

	if (!PlayCueSpec.GameplayCueTag.IsValid())
	{
		return Result;
	}

	UAbilitySystemComponent* TargetASC = Context.TargetAbilitySystemComponent.Get();
	UAbilitySystemComponent* SourceASC = Context.SourceAbilitySystemComponent.Get();

	FGameplayCueParameters CueParams;

	if (Context.bHasHitResult)
	{
		CueParams.Location = Context.HitResult.ImpactPoint;
	}
	else if (AActor* CauserActor = Cast<AActor>(Context.DamageCauser.Get()))
	{
		CueParams.Location = CauserActor->GetActorLocation();
	}
	else if (const IAttackContextProvider* Provider = Cast<IAttackContextProvider>(Context.DamageCauser.Get()))
	{
		CueParams.Location = Provider->GetAttackOrigin();
	}
	else if (Context.TargetActor.IsValid())
	{
		CueParams.Location = Context.TargetActor->GetActorLocation();
	}
	else
	{
		CueParams.Location = FVector::ZeroVector;
	}

	CueParams.Normal = Context.bHasHitResult
		? Context.HitResult.ImpactNormal
		: FVector::UpVector;

	CueParams.Instigator = Context.SourceActor.Get();
	CueParams.EffectCauser = Cast<AActor>(Context.DamageCauser.Get());
	CueParams.SourceObject = Context.DamageCauser.Get();

	if (TargetASC)
	{
		TargetASC->ExecuteGameplayCue(PlayCueSpec.GameplayCueTag, CueParams);
	}
	else if (SourceASC)
	{
		SourceASC->ExecuteGameplayCue(PlayCueSpec.GameplayCueTag, CueParams);
	}

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteKnockback(const FSpellKnockbackActionSpec& KnockbackSpec, const FSpellContext& Context)
{
	FSpellActionExecutionResult Result;

	if (CheckImmunity(FMagicultGameplayTags::Get().Action_Knockback, Context))
		return FSpellActionExecutionResult(false);

	AActor* TargetActor = ResolveActorByPolicy(KnockbackSpec.TargetPolicy, Context);
	if (!TargetActor)
	{
		return FSpellActionExecutionResult(false);
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	// DamageCauser 정책이면 UObject causer(폭발)의 GetAttackOrigin으로 중심을 얻는다(공용 헬퍼).
	const FVector OriginLocation = ResolveOriginByPolicy(KnockbackSpec.OriginPolicy, Context, FGameplayTag());

	FVector KnockbackDirection = FVector::ZeroVector;

	if (KnockbackSpec.bRadialFromOrigin)
	{
		KnockbackDirection = TargetLocation - OriginLocation;
	}
	else
	{
		if (AActor* DamageCauserActor = Cast<AActor>(Context.DamageCauser.Get()))
		{
			KnockbackDirection = DamageCauserActor->GetActorForwardVector();
		}
		else if(Context.SourceActor.IsValid())
		{
			KnockbackDirection = Context.SourceActor->GetActorForwardVector();
		}
	}

	if (KnockbackSpec.bNormalizeHorizontalDirection)
	{
		KnockbackDirection.Z = 0.f;
	}

	if (!KnockbackDirection.Normalize())
	{
		return FSpellActionExecutionResult(false);
	}

	// Character(플레이어/몬스터)면 속도 기반 넉백 적용
	AMagicultCharacterBase* Character = Cast<AMagicultCharacterBase>(TargetActor);
	UCharacterMovementComponent* MoveComp = Character ? Character->GetCharacterMovement() : nullptr;
	bool bDead = Character ? ICombatInterface::Execute_IsDead(Character) : false;

	if (Character && MoveComp && !bDead)
	{
		// GameplayEvent로 KnockbackGameplayAbility(ServerOnly)를 트리거한다.
		// LaunchCharacter는 서버에서만 실행되고 이동 리플리케이션으로 클라에 반영된다.
		if (UAbilitySystemComponent* TargetASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
		{
			const FVector Force = (KnockbackDirection * KnockbackSpec.Strength)
				+ (FVector::UpVector * KnockbackSpec.UpwardStrength);

			FGameplayAbilityTargetData_Knockback* Data = new FGameplayAbilityTargetData_Knockback();
			Data->Force = Force;

			FGameplayEventData Payload;
			Payload.EventTag = FMagicultGameplayTags::Get().Action_Knockback;
			Payload.Instigator = Context.SourceActor.Get();
			Payload.Target = TargetActor;
			Payload.TargetData.Add(Data);

			TargetASC->HandleGameplayEvent(Payload.EventTag, &Payload);

			Result.ResultTag = FMagicultGameplayTags::Get().Action_Knockback;
			return Result;
		}
		return Result;
	}

	// 물리 오브젝트면 impulse fallback
	if (KnockbackSpec.bAffectPhysicsBodies)
	{
		const FVector Impulse =
			(KnockbackDirection * KnockbackSpec.Strength) +
			(FVector::UpVector * KnockbackSpec.UpwardStrength);

		if (Character && bDead)
		{
			// 래그돌 impulse를 모든 머신에 멀티캐스트(서버 전용 AddImpulse는 클라 시체가 안 날아감).
			FName BoneName = Cast<ICombatInterface>(Character)->GetDefaultKnockbackBone();
			Character->MulticastApplyRagdollImpulse(Impulse, BoneName);
		}
		else if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent()))
		{
			if (PrimitiveComp->IsSimulatingPhysics())
			{
				PrimitiveComp->AddImpulse(Impulse, NAME_None, true);
			}
		}
	}

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteInterrupt(
	const FSpellInterruptActionSpec& InterruptSpec, const FSpellContext& Context)
{
	UAbilitySystemComponent* TargetASC = Context.TargetAbilitySystemComponent.Get();
	if (!TargetASC || InterruptSpec.CancelTags.IsEmpty()) return FSpellActionExecutionResult(false);

	if (InterruptSpec.InterruptTag.IsValid() && CheckImmunity(InterruptSpec.InterruptTag, Context))
		return FSpellActionExecutionResult(false);

	TargetASC->CancelAbilities(&InterruptSpec.CancelTags);
	return FSpellActionExecutionResult(true);
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteDestroyActor(const FSpellContext& Context)
{
	FSpellActionExecutionResult Result;

	if (UCombatSubsystem* CombatSubsystem = GetWorld()->GetSubsystem<UCombatSubsystem>())
	{
		CombatSubsystem->RegisterPendingCleanupObject(Context.DamageCauser.Get());
	}
	return Result;
}
