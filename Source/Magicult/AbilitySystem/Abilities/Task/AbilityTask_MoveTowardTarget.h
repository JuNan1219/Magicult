// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Navigation/PathFollowingComponent.h"
#include "AbilityTask_MoveTowardTarget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMoveTowardTargetDelegate, FVector, MoveLocation);

/**
 * 
 */
UCLASS()
class MAGICULT_API UAbilityTask_MoveTowardTarget : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_MoveTowardTarget();

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (DisplayName = "Move Toward Target", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_MoveTowardTarget* MoveTowardTarget(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		FVector InStartLocation,
		FVector InTargetLocation,
		float InMoveDistance,
		float InAcceptanceRadius = 30.f,
		bool bInDrawDebug = false,
		float InStopDistanceFromWall = 50.f
	);

	UPROPERTY(BlueprintAssignable)
	FMoveTowardTargetDelegate OnRequestSuccess;

	UPROPERTY(BlueprintAssignable)
	FMoveTowardTargetDelegate OnMoveFinished;

	UPROPERTY(BlueprintAssignable)
	FMoveTowardTargetDelegate OnMoveFailed;

	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
	void HandleMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);

	void StopMovement();
	bool ResolveMoveLocation(FVector& OutMoveLocation) const;

protected:
	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY()
	float MoveDistance = 0.f;

	UPROPERTY()
	float AcceptanceRadius = 30.f;

	// 벽에 딱 붙지 않도록 목표 지점을 벽 표면에서 이만큼 앞에서 자른다. 호출자가 캡슐 반지름 등을 고려해 조정 가능.
	UPROPERTY()
	float StopDistanceFromWall = 50.f;

	TWeakObjectPtr<class APawn> CachedPawn;
	TWeakObjectPtr<class AAIController> CachedAIController;

	FDelegateHandle MoveFinishedDelegateHandle;
	FAIRequestID ActiveMoveRequestID;
	bool bMoveRequested = false;
	bool bStoppedMovement = false;

	UPROPERTY()
	FVector FinalMoveLocation = FVector::ZeroVector;

	bool bDrawDebug = false;

private:
	void CleanupMoveDelegate();
};
