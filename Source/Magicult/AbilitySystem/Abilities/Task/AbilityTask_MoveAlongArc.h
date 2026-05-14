// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_MoveAlongArc.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMoveAlongArcSimpleDelegate);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MAGICULT_API UAbilityTask_MoveAlongArc : public UAbilityTask
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintAssignable)
    FMoveAlongArcSimpleDelegate OnEndPhase;

    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "Move Along Arc", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
    static UAbilityTask_MoveAlongArc* MoveAlongArc(UGameplayAbility* OwningAbility, FVector InStartLocation, FVector InEndLocation, float InDuration, float InArcHeight, float EndPhaseThreshold);

    virtual void Activate() override;
    virtual void TickTask(float DeltaTime) override;
    virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
    FVector EvaluatePosition(float Alpha) const;


    FVector StartLocation = FVector::ZeroVector;
    FVector EndLocation = FVector::ZeroVector;
    FVector ApexLocation = FVector::ZeroVector;

    float Duration = 1.0f;
    float ElapsedTime = 0.0f;
    float ArcHeight = 300.0f;

    float EndPhaseThreshold = 0.8f;

    bool bIsEndPhase = false;
};
