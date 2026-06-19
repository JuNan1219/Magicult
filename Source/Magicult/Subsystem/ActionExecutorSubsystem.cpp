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
#include "Components/PrimitiveComponent.h"
#include "Magicult/Actor/MagicultProjectile.h"
#include "Magicult/Subsystem/CombatResolveSubsystem.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/Interface/ElementCarrierInterface.h"
#include "Magicult/Subsystem/CombatSubsystem.h"
#include "Magicult/Game/MagicultGameInstance.h"
#include "Magicult/Data/ImmunityPolicy.h"

TArray<FSpellTriggerEventData> UActionExecutorSubsystem::ExecuteRequest(const TArray<FSpellActionExecutionRequest>& Request)
{
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

		FSpellActionExecutionResult Result = ExecuteSingleAction(CurrentAction, CurrentContext);

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
	// 액션 권한 분리:
	//   코스메틱(Niagara·Cue) - 서버/클라 모두 실행하되, 전용 서버는 렌더러가 없으므로 스킵
	//   권위(Damage·GE·Knockback 등) - 서버 전용. 클라이언트는 게임 상태 변경 불가
	// TriggerQueue 자체는 서버·클라이언트 양쪽에서 처리되며,
	// 각 머신은 자신에게 허용된 액션 타입만 실행한다.
	const bool bIsCosmetic = Action.ActionType == ESpellActionType::EmitNiagara
	                      || Action.ActionType == ESpellActionType::PlayCue;

	if (bIsCosmetic && GetWorld()->IsNetMode(NM_DedicatedServer)) return {};
	if (!bIsCosmetic && GetWorld()->IsNetMode(NM_Client))         return {};

	switch (Action.ActionType)
	{
	case ESpellActionType::ConsumePierce:
		return ExecuteConsumePierce(Action.ConsumePierceSpec, Context);
	case ESpellActionType::ApplyDamage:
		return ExecuteApplyDamage(Context, false);
	case ESpellActionType::ApplyRadialDamage:
		return ExecuteRadialDamage(Action.RadialDamageSpec, Context);
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

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteApplyDamage(const FSpellContext& Context, bool IsRadialDamage)
{
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

	const bool bAppliedDamage = UMagicultAbilitySystemLibrary::ApplyDefaultDamageGameplayEffect(Context, SourceASC, TargetASC, IsRadialDamage);

	if (!bAppliedDamage)
	{
		return Result;
	}

	FSpellTriggerEventData TriggerData;
	TriggerData.TriggerTag = FMagicultGameplayTags::Get().Trigger_OnAttack;
	TriggerData.SpellContext = Context;

	if (IsRadialDamage)
		TriggerData.TriggerContextTags.AddTag(FMagicultGameplayTags::Get().AttackType_Radial);
	else
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

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteRadialDamage(const FSpellRadialDamageActionSpec& RadialDamageSpec, const FSpellContext& Context)
{
	FSpellActionExecutionResult Result;

	UWorld* World = GetWorld();
	if (!World || RadialDamageSpec.Radius <= 0.f)
	{
		return Result;
	}

	FVector Origin = FVector::ZeroVector;
	if (Context.bHasHitResult)
	{
		Origin = Context.HitResult.ImpactPoint;
	}
	else if (const AActor* DamageCauser = Cast<AActor>(Context.DamageCauser.Get()))
	{
		Origin = DamageCauser->GetActorLocation();
	}
	else if (const AActor* SourceActor = Context.SourceActor.Get())
	{
		Origin = SourceActor->GetActorLocation();
	}

	TArray<FOverlapResult> OverlapResults;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpellRadialDamage), false);
	if (Context.SourceActor.IsValid() && RadialDamageSpec.bIgnoreOwner)
	{
		QueryParams.AddIgnoredActor(Context.SourceActor.Get());
	}
	if (AActor* DamageCauserActor = Cast<AActor>(Context.DamageCauser.Get()))
	{
		QueryParams.AddIgnoredActor(DamageCauserActor);
	}

	const bool bHasOverlap = World->OverlapMultiByObjectType(
		OverlapResults,
		Origin,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllDynamicObjects,
		FCollisionShape::MakeSphere(RadialDamageSpec.Radius),
		QueryParams);

	bool bTargetEnemiesOnly = false;
	if (const IAttackContextProvider* Provider = Cast<IAttackContextProvider>(Context.DamageCauser.Get()))
	{
		bTargetEnemiesOnly = Provider->GetAttackSpec().bTargetEnemiesOnly;
	}

	if (bHasOverlap)
	{
		TSet<TWeakObjectPtr<AActor>> ProcessedActors;

		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			AActor* TargetActor = OverlapResult.GetActor();
			if (!TargetActor || ProcessedActors.Contains(TargetActor))
			{
				continue;
			}

			ProcessedActors.Add(TargetActor);

			if (bTargetEnemiesOnly)
			{
				AActor* SourceActor = Context.SourceActor.Get();
				if (SourceActor && !UMagicultAbilitySystemLibrary::IsNotFriend(SourceActor, TargetActor))
					continue;
			}

			UAbilitySystemComponent* TargetASC =
				UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
			if (!TargetASC)
			{
				continue;
			}

			FSpellContext RadialContext = Context;
			RadialContext.TargetActor = TargetActor;
			RadialContext.TargetAbilitySystemComponent = TargetASC;
			RadialContext.bHasHitResult = false;

			FSpellActionExecutionResult DamageResult = ExecuteApplyDamage(RadialContext, true);
			Result.GeneratedTriggerEvents.Append(DamageResult.GeneratedTriggerEvents);
			Result.GeneratedActionRequests.Append(DamageResult.GeneratedActionRequests);

			if (RadialDamageSpec.bApplyKnockback)
			{
				FSpellActionSpec KnockbackAction;
				KnockbackAction.ActionType = ESpellActionType::Knockback;
				KnockbackAction.KnockbackSpec = RadialDamageSpec.KnockbackSpec;

				Result.GeneratedActionRequests.Emplace(KnockbackAction, RadialContext);
			}
		}
	}

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteResolveElementInteraction(const FSpellApplyElementActionSpec& ApplyElementSpec, const FSpellContext& Context)
{
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
	FSpellActionExecutionResult Result;

	if (CheckImmunity(FMagicultGameplayTags::Get().Action_Debuff, Context))
		return FSpellActionExecutionResult(false);

	UAbilitySystemComponent* SourceASC = Context.SourceAbilitySystemComponent.Get();
	UAbilitySystemComponent* TargetASC = Context.TargetAbilitySystemComponent.Get();
	if (!SourceASC || !TargetASC)
	{
		return FSpellActionExecutionResult(false);
	}

	// 사망한 대상에게는 디버프 적용 안 함
	AActor* TargetActor = Context.TargetActor.Get();
	if (TargetActor && ICombatInterface::Execute_IsDead(TargetActor))
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

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteGameplayEffect(const FSpellApplyGEActionSpec& ApplyGESpec, const FSpellContext& Context)
{
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

	auto ResolveActorByPolicy = [&Context](ESpellActionTargetPolicy Policy) -> AActor*
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
			{
				if (AActor* DamageCauserActor = Cast<AActor>(Context.DamageCauser.Get()))
				{
					return DamageCauserActor;
				}
				return Context.TargetActor.Get();
			}

			default:
				return nullptr;
			}
		};

	auto ResolveTransformByPolicy = [&Context, &ResolveActorByPolicy, &EmitNiagaraSpec](ESpellActionTargetPolicy Policy) -> FTransform
		{
			switch (Policy)
			{
			case ESpellActionTargetPolicy::HitLocation:
				if (Context.bHasHitResult)
				{
					return FTransform(Context.HitResult.ImpactPoint);
				}
				break;
			case ESpellActionTargetPolicy::SocketTransform:
				if (AActor* Actor = ResolveActorByPolicy(Policy))
				{
					if (Actor->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()) && EmitNiagaraSpec.SocketTag.IsValid())
						return ICombatInterface::Execute_GetCombatSocketLocation(Actor, EmitNiagaraSpec.SocketTag);
					return Actor->GetActorTransform();
				}
				break;

			case ESpellActionTargetPolicy::Source:
			case ESpellActionTargetPolicy::Target:
			case ESpellActionTargetPolicy::HitActor:
			case ESpellActionTargetPolicy::DamageCauser:
				if (AActor* Actor = ResolveActorByPolicy(Policy))
				{
					return Actor->GetActorTransform();
				}
				break;

			default:
				break;
			}

			return FTransform::Identity;
		};

	const FTransform BaseTransform = ResolveTransformByPolicy(EmitNiagaraSpec.TargetPolicy);
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
	CueParams.Location = Context.bHasHitResult
		? Context.HitResult.ImpactPoint
		: (Context.TargetActor.IsValid()
			? Context.TargetActor->GetActorLocation()
			: FVector::ZeroVector);

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

	auto ResolveActorByPolicy = [&Context](ESpellActionTargetPolicy Policy) -> AActor*
		{
			switch (Policy)
			{
			case ESpellActionTargetPolicy::Source:
				return Context.SourceActor.Get();

			case ESpellActionTargetPolicy::Target:
			case ESpellActionTargetPolicy::HitActor:
				return Context.TargetActor.Get();

			case ESpellActionTargetPolicy::DamageCauser:
			{
				AActor* DamageCauserActor = Cast<AActor>(Context.DamageCauser.Get());
				return DamageCauserActor ? DamageCauserActor : Context.TargetActor.Get();
			}
			default:
				return nullptr;
			}
		};

	auto ResolveLocationByPolicy = [&Context, &ResolveActorByPolicy](ESpellActionTargetPolicy Policy) -> FVector
		{
			switch (Policy)
			{
			case ESpellActionTargetPolicy::HitLocation:
				if (Context.bHasHitResult)
				{
					return Context.HitResult.ImpactPoint;
				}

				break;

			case ESpellActionTargetPolicy::Source:
			case ESpellActionTargetPolicy::Target:
			case ESpellActionTargetPolicy::HitActor:
			case ESpellActionTargetPolicy::DamageCauser:
				if (AActor* Actor = ResolveActorByPolicy(Policy))
				{
					return Actor->GetActorLocation();
				}
				break;

			default:
				break;
			}

			return FVector::ZeroVector;
		};

	if (CheckImmunity(FMagicultGameplayTags::Get().Action_Knockback, Context))
		return FSpellActionExecutionResult(false);

	AActor* TargetActor = ResolveActorByPolicy(KnockbackSpec.TargetPolicy);
	if (!TargetActor)
	{
		return FSpellActionExecutionResult(false);
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector OriginLocation = ResolveLocationByPolicy(KnockbackSpec.OriginPolicy);

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

	// Character면 RootMotionSource 사용
	AMagicultCharacterBase* Character = Cast<AMagicultCharacterBase>(TargetActor);
	UCharacterMovementComponent* MoveComp = Character ? Character->GetCharacterMovement() : nullptr;
	bool bDead = Character ? ICombatInterface::Execute_IsDead(Character) : false;

	if (Character && MoveComp && !bDead)
	{
		FVector Force = (KnockbackDirection * KnockbackSpec.Strength)
			+ (FVector::UpVector * KnockbackSpec.UpwardStrength);

		TSharedPtr<FRootMotionSource_ConstantForce> RMS = MakeShared<FRootMotionSource_ConstantForce>();
		RMS->InstanceName = FName(TEXT("SpellKnockback"));
		RMS->AccumulateMode = ERootMotionAccumulateMode::Override;
		RMS->Priority = 500;
		RMS->Force = Force;
		RMS->Duration = 0.25f;
		RMS->StrengthOverTime = nullptr;
		// 넉백이 끝나고 관성 유지
		RMS->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;

		MoveComp->ApplyRootMotionSource(RMS);

		Result.ResultTag = FMagicultGameplayTags::Get().Action_Knockback;
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
			FName BoneName = Cast<ICombatInterface>(Character)->GetDefaultKnockbackBone();
			Character->GetMesh()->AddImpulse(Impulse, BoneName, true);
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
