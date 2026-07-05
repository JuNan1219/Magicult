// Fill out your copyright notice in the Description page of Project Settings.

#include "RadialAttackInstance.h"

#include "AbilitySystemBlueprintLibrary.h"
#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "RadialDamageConfig.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"

void URadialAttackInstance::InitializeWithConfig()
{
	if (const URadialDamageConfig* RadialConfig = Cast<URadialDamageConfig>(AttackConfig))
	{
		Radius = RadialConfig->Radius;
		bIgnoreOwner = RadialConfig->bIgnoreOwner;
	}
}

void URadialAttackInstance::SetAttackSpec(const FAttackSpec& InAttackSpec)
{
	Super::SetAttackSpec(InAttackSpec);

	// 여기까지 왔다는 건 부모 Spec을 물려받는 경로다(bOverride=true).
	// 폭발은 정지체이므로 발사체에서 딸려온 속도 기반 데미지를 끈다
	// (구 ExecuteRadialDamage의 IsRadialDamage=true와 동일 효과). 자기 기본 Spec을 쓰는
	// 경우(bOverride=false)엔 SetAttackSpec이 호출되지 않아 BP 설정이 그대로 존중된다.
	AttackSpec.bUseVelocityDamage = false;
}

void URadialAttackInstance::HandleEventTag_Implementation(FGameplayTag EventTag)
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

	const FGameplayTag TriggerHit = FMagicultGameplayTags::Get().Trigger_Hit;

	for (AActor* TargetActor : Targets)
	{
		UAbilitySystemComponent* TargetASC =
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!TargetASC)
		{
			continue;
		}

		// DamageCauser=this는 Context에 이미 실려 있음(SpawnAttackInstance가 세팅).
		FSpellContext HitContext = Context;
		HitContext.TargetActor = TargetActor;
		HitContext.TargetAbilitySystemComponent = TargetASC;
		HitContext.bHasHitResult = false;
		// causer가 발행하는 파생 트리거이므로 depth +1 (무한 연쇄 폭발 가드).
		HitContext.TriggerChainDepth = Context.TriggerChainDepth + 1;

		UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(this, TriggerHit, FGameplayTagContainer(), HitContext);
	}
}

bool URadialAttackInstance::TryCollectTargets(TArray<AActor*>& OutTargets) const
{
	UWorld* World = GetWorld();
	if (!World || Radius <= 0.f)
	{
		return false;
	}

	AActor* Source = Context.SourceActor.Get();
	const FVector Center = GetAttackOrigin();   // SpawnAttackInstance가 policy로 해석해 주입

	FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RadialAttackOverlap), false);
	if (bIgnoreOwner && Source)
	{
		Params.AddIgnoredActor(Source);
	}

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
	if (bDrawDebug) DrawDebugSphere(World, Center, Radius, 16, bHit ? FColor::Green : FColor::Red, false, 1.0f);
#endif

	if (!bHit)
	{
		return false;
	}

	const bool bEnemiesOnly = AttackSpec.bTargetEnemiesOnly;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* TargetActor = Overlap.GetActor();
		if (!TargetActor)
		{
			continue;
		}

		if (bEnemiesOnly && Source && !UMagicultAbilitySystemLibrary::IsNotFriend(Source, TargetActor))
		{
			continue;
		}

		OutTargets.AddUnique(TargetActor);
	}

	return OutTargets.Num() > 0;
}
