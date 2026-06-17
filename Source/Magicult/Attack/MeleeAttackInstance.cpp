// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeAttackInstance.h"

#include "AbilitySystemBlueprintLibrary.h"
#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "OverlapDamageConfig.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/Subsystem/CombatSubsystem.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"

void UMeleeAttackInstance::InitializeWithConfig()
{
	if (const UOverlapDamageConfig* OverlapConfig = Cast<UOverlapDamageConfig>(AttackConfig))
	{
		AttackRadius = OverlapConfig->CollisionConfig.SphereRadius;
	}
}

void UMeleeAttackInstance::HandleEventTag_Implementation(FGameplayTag EventTag)
{
	Super::HandleEventTag_Implementation(EventTag);

	if (!IsActive())
	{
		return;
	}
	
	if (!EventTag.MatchesTagExact(FMagicultGameplayTags::Get().Notify_Attack_Commit))
	{
		return;
	}

	TArray<AActor*> Targets;
	if (!TryCollectTargets(Targets))
	{
		return;
	}

	for (AActor* TargetActor : Targets)
	{
		if (!IsValidMeleeTarget(TargetActor))
		{
			continue;
		}

		FSpellContext HitContext = Context;
		HitContext.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		HitContext.TargetActor = TargetActor;

		UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(this, FMagicultGameplayTags::Get().Trigger_Hit, FGameplayTagContainer(), HitContext);

		HitActors.Add(TargetActor);
	}
}
bool UMeleeAttackInstance::TryCollectTargets(TArray<AActor*>& OutTargets) const
{
	AActor* Source = Context.SourceActor.Get();
	if (!Source)
	{
		return false;
	}

	UWorld* World = Source->GetWorld();
	if (!World)
	{
		return false;
	}

	FVector Center = FVector::ZeroVector;
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwnerActor()))
	{
		Center = ICombatInterface::Execute_GetCombatSocketLocation(GetOwnerActor(), SocketTag).GetLocation();
	}

	FCollisionShape Shape = FCollisionShape::MakeSphere(AttackRadius);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeAttackOverlap), false, Source);

	TArray<FOverlapResult> Overlaps;
	const bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		Center,
		FQuat::Identity,
		FCollisionObjectQueryParams::AllDynamicObjects,
		Shape,
		Params
	);

#if !UE_BUILD_SHIPPING
	if (bDrawDebug) DrawDebugSphere(World, Center, AttackRadius, 16, bHit ? FColor::Green : FColor::Red, false, 1.0f);
#endif

	if (!bHit)
	{
		return false;
	}

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* TargetActor = Result.GetActor();
		if (TargetActor)
		{
			OutTargets.AddUnique(TargetActor);
		}
	}

	return OutTargets.Num() > 0;
}
bool UMeleeAttackInstance::IsValidMeleeTarget(AActor* TargetActor) const
{
	AActor* Source = Context.SourceActor.Get();
	if (!TargetActor || !Source)
	{
		return false;
	}

	if (TargetActor == Source)
	{
		return false;
	}

	if (HitActors.Contains(TargetActor))
	{
		return false;
	}

	return true;
}

void UMeleeAttackInstance::FireHitTrigger(const FSpellContext& NewContext) const
{
	UCombatSubsystem* CombatSubsystem = GetWorld()->GetSubsystem<UCombatSubsystem>();
	if (!CombatSubsystem) return;

	FSpellTriggerEventData NewTriggerEventData;
	NewTriggerEventData.TriggerTag = FMagicultGameplayTags::Get().Trigger_Hit;
	NewTriggerEventData.SpellContext = NewContext;

	CombatSubsystem->Enqueue(NewTriggerEventData);
}