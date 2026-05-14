// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_Damage.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UExecCalc_Damage();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

    float CalcImpactDamage(
        const FVector& VelocityA,
        const FVector& VelocityB,
        float BaseDamage,
        float SpeedDivisor,
        float MinMultiplier = 0.5f,
        float MaxMultiplier = 5.0f) const;
};
