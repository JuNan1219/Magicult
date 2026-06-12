#pragma once

#include "GameplayTagContainer.h"

#include "AttackSpec.generated.h"

USTRUCT(BlueprintType)
struct FAttackSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ElementTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElementRatio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseVelocityDamage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageModifier = 1.0f;

	// true면 아군을 피아식별해서 공격하지 않는다. false(기본)면 아군 포함 모두 공격.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTargetEnemiesOnly = false;
};