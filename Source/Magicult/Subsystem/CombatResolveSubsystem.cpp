// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatResolveSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Magicult/Component/MagicComponent.h"
#include "Magicult/Data/ProjectileTypes.h"
#include "Magicult/Data/InterruptPolicy.h"
#include "Magicult/Game/MagicultGameInstance.h"

#include "Magicult/MagicultGameplayTags.h"



FResolvedSpellProjectileResult UCombatResolveSubsystem::ResolveProjectile(const FSpellContext& InSpellContext, const FSpellState& InSpellState) const
{
	FResolvedSpellProjectileResult Result;

	Result.Spec = ResolveProjectileSpec(InSpellContext, InSpellState, Result.RuntimeState);

	return Result;
}

FResolvedSpellProjectileSpec UCombatResolveSubsystem::ResolveProjectileSpec(const FSpellContext& InSpellContext, const FSpellState& InSpellState, FSpellRuntimeState& OutRuntimeState) const
{
	FResolvedSpellProjectileSpec Resolved;
	TArray<FSpellElementSpec> ElementData;
	UMagicultAbilitySystemLibrary::GetElementData(this, ElementData);

	TArray<FSpellProjectileFeatureSpec> AmmoTypeData;
	UMagicultAbilitySystemLibrary::GetAmmoTypeData(this, AmmoTypeData);

	TArray<FSpellProjectileClassEntry> ProjectileBaseClasses;
	UMagicultAbilitySystemLibrary::GetProjectileBaseClass(this, ProjectileBaseClasses);


	if (ElementData.IsEmpty() || AmmoTypeData.IsEmpty()) return Resolved;

	Resolved.ElementTag = InSpellState.ElementTagPair.Tag;
	Resolved.AmmoTypeTag = InSpellState.AmmoTypeTagPair.Tag;
	// ----------------------------------------------------------------
	// 1) Speed, Direction 태그 기반 Resolve
	// ----------------------------------------------------------------

	Resolved.InitialSpeed = UMagicultAbilitySystemLibrary::GetTagToFloatData(this, InSpellState.SpeedTag);

	AActor* SourceActor = nullptr;
	if (UAbilitySystemComponent* SourceASC = InSpellContext.SourceAbilitySystemComponent.Get())
	{
		SourceActor = SourceASC->GetAvatarActor();
		if (!SourceActor)
		{
			SourceActor = SourceASC->GetOwnerActor();
		}
	}
	if (SourceActor)
	{
		const FVector Forward = SourceActor->GetActorForwardVector();
		const FVector Right = SourceActor->GetActorRightVector();

		float UpAngle = UMagicultAbilitySystemLibrary::GetTagToFloatData(this, InSpellState.DirectionTag);
		if (UpAngle < 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("you don't have data %s :"), *InSpellState.DirectionTag.ToString());
		}
		Resolved.LaunchDirection = Forward.RotateAngleAxis(-UpAngle, Right).GetSafeNormal();
	}
	else
	{
		Resolved.LaunchDirection = FVector::ForwardVector;
	}

	
	// ----------------------------------------------------------------
	// 2) ElementData 기반 Resolve
	// ----------------------------------------------------------------
	if (Resolved.ElementTag.IsValid())
	{
		const FSpellElementSpec* FoundElementSpec =
			ElementData.FindByPredicate([&](const FSpellElementSpec& Spec)
				{
					return Spec.ElementTag == Resolved.ElementTag;
				});

		if (FoundElementSpec != nullptr)
		{
			Resolved.ElementRatio = FoundElementSpec->ElementRatio;
			Resolved.ProjectileCollider = FoundElementSpec->ProjectileCollider;
			Resolved.ProjectileNiagara = FoundElementSpec->ElementNiagara;
			Resolved.TriggerActionBindings.Append(FoundElementSpec->HitImpactTriggerAction);

			if (Resolved.AmmoTypeTag == FMagicultGameplayTags::Get().Magic_AmmoType_Shield && FoundElementSpec->ShieldSpec.ShieldNiagara)
			{
				Resolved.ProjectileCollider = FoundElementSpec->ShieldSpec.ShieldCollider;
				Resolved.ProjectileNiagara = FoundElementSpec->ShieldSpec.ShieldNiagara;
			}
		}
	}

	// ----------------------------------------------------------------
	// 3) AmmoTypeData 기반 Resolve
	// ----------------------------------------------------------------
	
	if (Resolved.AmmoTypeTag.IsValid())
	{
		const FSpellProjectileFeatureSpec* AmmoSpec = AmmoTypeData.FindByPredicate(
			[&](const FSpellProjectileFeatureSpec& Spec)
			{
				return Spec.FeatureTag == Resolved.AmmoTypeTag;
			});

		if (AmmoSpec)
		{
			Resolved.CollisionEventPolicy = AmmoSpec->CollisionEventPolicy;
			Resolved.TriggerActionBindings.Append(AmmoSpec->TriggerActionBindings);

			switch (AmmoSpec->FeatureType)
			{
			case ESpellProjectileFeatureType::FastShot:
				Resolved.InitialSpeed += AmmoSpec->FastShotSpec.AdditionalSpeed;
				break;
			case ESpellProjectileFeatureType::Pierce:
				Resolved.bCanHitSameTargetAgain = AmmoSpec->PierceSpec.bCanHitSameTargetAgain;
				Resolved.RehitCooldown = AmmoSpec->PierceSpec.RehitCooldown;
				OutRuntimeState.RemainingPierceCount = AmmoSpec->PierceSpec.MaxPierceCount;
				break;
			case ESpellProjectileFeatureType::Bounce:
				OutRuntimeState.RemainingBounceCount = AmmoSpec->BounceSpec.MaxBounceCount;
				Resolved.CollisionEventPolicy = ESpellCollisionEventPolicy::Bounce;
				Resolved.Bounciness = AmmoSpec->BounceSpec.Restitution;
				break;
			case ESpellProjectileFeatureType::Explosive:
				if (AmmoSpec->ExplosionSpec.CommonActionSet)
					Resolved.TriggerActionBindings.Append(AmmoSpec->ExplosionSpec.CommonActionSet->Bindings);
				if (const TObjectPtr<UTriggerActionSet>* Found = AmmoSpec->ExplosionSpec.ElementActionSets.Find(Resolved.ElementTag))
					if (*Found)
						Resolved.TriggerActionBindings.Append((*Found)->Bindings);
				break;
			default:
				break;
			}
		}
	}

	// ----------------------------------------------------------------
	// 4) Resolve 확인 — ProjectileBaseClass
	// ----------------------------------------------------------------
	const FSpellProjectileClassEntry* ProjectileClass = ProjectileBaseClasses.FindByPredicate(
		[&](const FSpellProjectileClassEntry& Entry)
		{
			return Entry.Shape == Resolved.ProjectileCollider.Shape;
		});
	if (ProjectileClass)
	{
		Resolved.ProjectileBaseClass = ProjectileClass->ProjectileClass;
	}

	// ----------------------------------------------------------------
	// 5) EffectTags → Modifier 적용
	// ----------------------------------------------------------------
	TArray<FSpellModifierConfig> ModifierConfigs;
	UMagicultAbilitySystemLibrary::GetModifierConfigs(this, ModifierConfigs);

	for (const FSpellTagLevelPair& EffectPair : InSpellState.EffectTags)
	{
		const FGameplayTag& Tag = EffectPair.Tag;
		const FSpellModifierConfig* Config = ModifierConfigs.FindByPredicate(
			[&](const FSpellModifierConfig& C) { return C.ModifierTag == Tag; });
		if (!Config) continue;

		switch (Config->ModifierType)
		{
		case ESpellModifierType::SizeUp:
			Resolved.SizeScale *= Config->SizeScale;
			break;
		case ESpellModifierType::LongLife:
			Resolved.LifeSpanMultiplier *= Config->LifeSpanMultiplier;
			break;
		case ESpellModifierType::ZeroGravity:
			Resolved.GravityScale = FMath::Min(Resolved.GravityScale, Config->GravityScaleOverride);
			break;
		case ESpellModifierType::Friendly:
			Resolved.bTargetEnemiesOnly = true;
			break;
		case ESpellModifierType::Tracking:
			if (!Resolved.bTracking)
			{
				Resolved.bTracking = true;
				Resolved.TrackingSearchRadius = Config->TrackingSpec.SearchRadius;
				Resolved.HomingAccelerationMagnitude = Config->TrackingSpec.HomingAccelerationMagnitude;
			}
			break;
		default:
			break;
		}
	}

	return Resolved;
}


FElementInteractionResolveResult UCombatResolveSubsystem::ResolveIncomingElement(const FSpellContext& InSpellContext, const FGameplayTag& IncomingElementTag)
{
	FElementInteractionResolveResult ElementInteractionResolveResult;

	const bool bShouldApplyDefaultDebuff = ResolveElementInteraction(InSpellContext, IncomingElementTag,ElementInteractionResolveResult);
	if (bShouldApplyDefaultDebuff)
	{
		ResolveElementDebuff(InSpellContext, IncomingElementTag, ElementInteractionResolveResult);
	}

	return ElementInteractionResolveResult;
}

bool UCombatResolveSubsystem::ResolveElementInteraction(const FSpellContext& InSpellContext,const FGameplayTag& IncomingElementTag , FElementInteractionResolveResult& ElementInteractionResolveResult)
{
	UAbilitySystemComponent* TargetASC = InSpellContext.TargetAbilitySystemComponent.Get();

	if (!IncomingElementTag.IsValid() || !TargetASC)
	{
		return false;
	}

	FGameplayTagContainer TargetOwnedTags;
	TargetASC->GetOwnedGameplayTags(TargetOwnedTags);

	TArray<FElementReactionRule> ReactionRules;

	UMagicultAbilitySystemLibrary::GetElementReactionRules(this, ReactionRules);

	// Priority 내림차순 정렬. StableSort라서 동순위 규칙은 DataAsset 배열 순서를 유지한다.
	ReactionRules.StableSort([](const FElementReactionRule& A, const FElementReactionRule& B)
		{
			return A.Priority > B.Priority;
		});

	for (const FElementReactionRule& Rule : ReactionRules)
	{
		if (!Rule.IncomingElementTag.IsValid()
			|| !Rule.ExistingTag.IsValid())
		{
			continue;
		}

		if (Rule.IncomingElementTag != IncomingElementTag)
		{
			continue;
		}

		if (!TargetOwnedTags.HasTagExact(Rule.ExistingTag))
		{
			continue;
		}

		for (const auto& Action : Rule.InteractionActions)
		{
			ElementInteractionResolveResult.InteractionActionRequests.Emplace(Action, InSpellContext);
		}

		if(Rule.bRemoveExistingTag)
		{
			ElementInteractionResolveResult.RemoveTags.AddUnique(Rule.ExistingTag);
		}
		// 상호작용은 한 번에 하나만 허용. Priority 내림차순으로 첫 매칭 규칙만 적용된다.
		return Rule.bApplyDebuff;
	}

	return true;
}

void UCombatResolveSubsystem::ResolveElementDebuff(const FSpellContext& InSpellContext,const  FGameplayTag& IncomingElementTag, FElementInteractionResolveResult& ElementInteractionResolveResult)
{
	if (!IncomingElementTag.IsValid())
	{
		return;
	}

	TArray<FDebuffRule> DebuffRules;

	UMagicultAbilitySystemLibrary::GetDebuffRules(this, DebuffRules);

	for (const FDebuffRule& Rule : DebuffRules)
	{
		if (!Rule.IncomingElementTag.IsValid())
		{
			continue;
		}

		if (!Rule.DebuffTag.IsValid())
		{
			continue;
		}

		if (Rule.IncomingElementTag != IncomingElementTag)
		{
			continue;
		}

		FSpellDebuffActionSpec DebuffSpec;
		DebuffSpec.DebuffTag = Rule.DebuffTag;
		DebuffSpec.EffectSpecs = Rule.EffectSpecs;

		FSpellActionSpec DebuffAction;
		DebuffAction.ActionType = ESpellActionType::ApplyDebuff;
		DebuffAction.DebuffSpec = DebuffSpec;

		ElementInteractionResolveResult.DebuffActionRequests.Emplace(DebuffAction, InSpellContext);
	}
}

TArray<FSpellActionExecutionRequest> UCombatResolveSubsystem::ResolveInterrupt(
	const FGameplayTag& ActionResultTag, const FSpellContext& Context) const
{
	TArray<FSpellActionExecutionRequest> Requests;

	const UMagicultGameInstance* GI = GetWorld()->GetGameInstance<UMagicultGameInstance>();
	if (!GI || !GI->InterruptPolicy)
	{
		UE_LOG(LogTemp, Error, TEXT("[InterruptPolicy] GameInstance에 InterruptPolicy가 설정되지 않았습니다. 인터럽트 없음으로 처리합니다."));
		return Requests;
	}

	for (const FInterruptRule& Rule : GI->InterruptPolicy->InterruptRules)
	{
		if (Rule.TriggerActionTag != ActionResultTag) continue;

		FSpellActionSpec InterruptAction;
		InterruptAction.ActionType = ESpellActionType::Interrupt;
		InterruptAction.InterruptSpec = Rule.InterruptSpec;
		Requests.Emplace(InterruptAction, Context);
	}

	return Requests;
}
