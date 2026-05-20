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
};