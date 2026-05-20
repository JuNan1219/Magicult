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

TArray<FSpellTriggerEventData> UActionExecutorSubsystem::ExecuteRequest(const TArray<FSpellActionExecutionRequest>& Request)
{
	TArray<FSpellTriggerEventData> ResultTriggerEventData;

	TQueue<FSpellActionExecutionRequest> PendingExecutionQueue;

	for (const FSpellActionExecutionRequest& ExecutionRequest : Request)
	{
		PendingExecutionQueue.Enqueue(ExecutionRequest);
	}

	int32 ChainDepth = 0;
	const int32 MaxChainDepth = 8;

	while (!PendingExecutionQueue.IsEmpty())
	{
		if (ChainDepth > MaxChainDepth)
		{
			UE_LOG(LogTemp, Warning, TEXT("SpellExecutor: MaxChainDepth exceeded."));
			break;
		}

		FSpellActionExecutionRequest CurrentExecutionRequest;
		PendingExecutionQueue.Dequeue(CurrentExecutionRequest);
		++ChainDepth;

		const FSpellActionSpec& CurrentAction = CurrentExecutionRequest.Action;
		const FSpellContext& CurrentContext = CurrentExecutionRequest.Context;

		FSpellActionExecutionResult Result = ExecuteSingleAction(CurrentAction, CurrentContext);

		for (const FSpellActionExecutionRequest& GeneratedRequest : Result.GeneratedActionRequests)
		{
			PendingExecutionQueue.Enqueue(GeneratedRequest);
		}

		if (Result.GeneratedTriggerEvents.Num() > 0)
		{
			ResultTriggerEventData.Append(Result.GeneratedTriggerEvents);
		}

		if (Result.bStopExecution)
		{
			break;
		}
	}

	return ResultTriggerEventData;
}

const UActionExecutionConfig* UActionExecutorSubsystem::GetActionExecutionConfig() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("World를 찾을 수 없습니다."));
		return nullptr;
	}

	const UMagicultGameInstance* MagicultGameInstance = World->GetGameInstance<UMagicultGameInstance>();
	if (!MagicultGameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("MyGameInstance를 찾을 수 없습니다."));
		return nullptr;
	}

	if (!MagicultGameInstance->ActionExecutionConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionExecutionConfig가 GameInstance에 설정되어 있지 않습니다."));
		return nullptr;
	}

	ActionExecutionConfig = MagicultGameInstance->ActionExecutionConfig;
	return ActionExecutionConfig;
}

bool UActionExecutorSubsystem::CanPassRequiredTags(const UAbilitySystemComponent* ASC, const FGameplayTagContainer& RequiredTags) const
{
	// RequiredTags가 없으면 ASC 없어도 통과
	if (RequiredTags.IsEmpty())
	{
		return true;
	}

	// RequiredTags가 있는데 ASC가 없으면 검사 불가능하므로 실패
	if (!ASC)
	{
		return false;
	}

	return ASC->HasAllMatchingGameplayTags(RequiredTags);
}

bool UActionExecutorSubsystem::CanPassBlockedTags(const UAbilitySystemComponent* ASC, const FGameplayTagContainer& BlockedTags) const
{
	// ASC가 없으면 막을 태그도 없으므로 통과
	if (!ASC)
	{
		return true;
	}

	// BlockedTags가 비어 있으면 당연히 통과
	if (BlockedTags.IsEmpty())
	{
		return true;
	}

	// 하나라도 걸리면 실패
	return !ASC->HasAnyMatchingGameplayTags(BlockedTags);
}

bool UActionExecutorSubsystem::CanExecuteAction(ESpellActionType ActionType, const FSpellContext& Context) const
{
	const UActionExecutionConfig* Config = GetActionExecutionConfig();
	if (!Config)
	{
		return false;
	}

	FActionConfig ActionConfig;
	if (!Config->FindActionConfig(ActionType, ActionConfig))
	{
		// 제약이 없으면 바로 통과
		return true;
	}

	const UAbilitySystemComponent* SourceASC =
		Context.SourceAbilitySystemComponent.Get();

	const UAbilitySystemComponent* TargetASC =
		Context.TargetAbilitySystemComponent.Get();

	// Source Required Tags
	if (!CanPassRequiredTags(SourceASC, ActionConfig.RequiredSourceTags))
	{
		return false;
	}

	// Target Required Tags
	if (!CanPassRequiredTags(TargetASC, ActionConfig.RequiredTargetTags))
	{
		return false;
	}

	// Source Blocked Tags
	if (!CanPassBlockedTags(SourceASC, ActionConfig.BlockedSourceTags))
	{
		return false;
	}

	// Target Blocked Tags
	if (!CanPassBlockedTags(TargetASC, ActionConfig.BlockedTargetTags))
	{
		return false;
	}

	return true;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteSingleAction(const FSpellActionSpec& Action, const FSpellContext& Context)
{
	const bool bIsCosmetic = Action.ActionType == ESpellActionType::EmitNiagara
	                      || Action.ActionType == ESpellActionType::PlayCue;

	// Cosmetic: Dedicated Server는 렌더러 없음 + GAS Cue 중복 전달 방지
	if (bIsCosmetic && GetWorld()->IsNetMode(NM_DedicatedServer)) return {};
	// Authoritative: 클라이언트는 게임 상태 변경 불가
	if (!bIsCosmetic && GetWorld()->IsNetMode(NM_Client))         return {};

	if (!CanExecuteAction(Action.ActionType, Context))
	{
		UE_LOG(LogTemp, Warning, TEXT("Action Tag검사에 실패했습니다. %s"), *UEnum::GetValueAsString(Action.ActionType));
		return FSpellActionExecutionResult();
	}

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

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteConsumePierce(const FSpellConsumePierceActionSpec& ConsumePierceSpec,	const FSpellContext& Context)
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

	UAbilitySystemComponent* SourceASC = Context.SourceAbilitySystemComponent.Get();
	UAbilitySystemComponent* TargetASC = Context.TargetAbilitySystemComponent.Get();
	if (!TargetASC)
	{
		return Result;
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
			return Result;
		}

		FSpellActionSpec SpellAction;
		SpellAction.ActionType = ESpellActionType::ApplyElement;
		SpellAction.ApplyElementSpec.IncomingElement = AttackContextProvider->GetAttackSpec().ElementTag;

		Result.GeneratedActionRequests.Emplace(SpellAction, Context);
	}

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

			Result.bStopExecution |= DamageResult.bStopExecution;
		}
	}

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteResolveElementInteraction(const FSpellApplyElementActionSpec& ApplyElementSpec, const FSpellContext& Context)
{
	FSpellActionExecutionResult Result;

	UCombatResolveSubsystem* CombatresolveSubsystem = GetWorld()->GetSubsystem<UCombatResolveSubsystem>();
	if (!CombatresolveSubsystem)
	{
		return Result;
	}
	
	FElementInteractionResolveResult ElementInteractionResolveResult = CombatresolveSubsystem->ResolveIncomingElement(Context, ApplyElementSpec.IncomingElement);

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

	UAbilitySystemComponent* SourceASC = Context.SourceAbilitySystemComponent.Get();
	UAbilitySystemComponent* TargetASC = Context.TargetAbilitySystemComponent.Get();
	if (!SourceASC || !TargetASC)
	{
		return Result;
	}

	for (const FDebuffEffectEntry& EffectEntry : DebuffSpec.EffectSpecs)
	{
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
			// 기존 동일 디버프 제거 후 갱신
			FGameplayTagContainer TagsToRemove;
			TagsToRemove.AddTag(DebuffSpec.DebuffTag);
			TargetASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);

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

	return Result;
}

FSpellActionExecutionResult UActionExecutorSubsystem::ExecuteSpawnActor(const FSpellSpawnActorActionSpec& SpawnActorSpec, const FSpellContext& Context)
{
	FSpellActionExecutionResult Result;

	UWorld* World = GetWorld();
	if (!World || SpawnActorSpec.ActorClass.IsNull())
	{
		return Result;
	}

	UClass* ActorClass = SpawnActorSpec.ActorClass.LoadSynchronous();
	if (!ActorClass)
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
	else if (AActor* DamageCaurserActor = Cast<AActor>(Context.DamageCauser.Get()))
	{
		BaseTransform = DamageCaurserActor->GetActorTransform();
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

	AActor* TargetActor = ResolveActorByPolicy(KnockbackSpec.TargetPolicy);
	if (!TargetActor)
	{
		return Result;
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
		return Result;
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
		RMS->AccumulateMode = ERootMotionAccumulateMode::Additive;
		RMS->Priority = 500;
		RMS->Force = Force;
		RMS->Duration = 0.1f;
		RMS->StrengthOverTime = nullptr;
		RMS->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::ClampVelocity;
		RMS->FinishVelocityParams.ClampVelocity = 0.f;
		RMS->FinishVelocityParams.SetVelocity = FVector::ZeroVector;

		MoveComp->ApplyRootMotionSource(RMS);

		if (!KnockbackSpec.InterruptSpec.CancelTags.IsEmpty())
		{
			FSpellActionSpec InterruptAction;
			InterruptAction.ActionType = ESpellActionType::Interrupt;
			InterruptAction.InterruptSpec = KnockbackSpec.InterruptSpec;
			Result.GeneratedActionRequests.Emplace(InterruptAction, Context);
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
	if (!TargetASC || InterruptSpec.CancelTags.IsEmpty()) return {};

	TargetASC->CancelAbilities(&InterruptSpec.CancelTags);
	return {};
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
