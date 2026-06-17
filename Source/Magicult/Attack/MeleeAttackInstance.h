// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackInstance.h"
#include "OverlapDamageConfig.h"
#include "MeleeAttackInstance.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UMeleeAttackInstance : public UAttackInstance
{
	GENERATED_BODY()
	
public:
	virtual void HandleEventTag_Implementation(const FGameplayTag& EventTag) override;

protected:
	virtual void InitializeWithConfig() override;

	bool TryCollectTargets(TArray<AActor*>& OutTargets) const;
	bool IsValidMeleeTarget(AActor* TargetActor) const;
	void FireHitTrigger(const FSpellContext& NewContext) const;

protected:
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = false;

	float AttackRadius = 40.f;

	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
