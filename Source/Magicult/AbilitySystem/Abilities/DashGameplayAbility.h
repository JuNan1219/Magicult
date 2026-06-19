// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultGameplayAbility.h"
#include "DashGameplayAbility.generated.h"

class UAbilityTask_ApplyRootMotionMoveToForce;
class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;

UCLASS()
class MAGICULT_API UDashGameplayAbility : public UMagicultGameplayAbility
{
	GENERATED_BODY()

public:
	UDashGameplayAbility();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UFUNCTION() void OnDashFinished();
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageInterrupted();
	UFUNCTION() void OnMontageCancelled();

private:
	void TerminateAbility(bool bCancelled);
	FVector ResolveDashDirection(const FGameplayEventData* TriggerEventData) const;

	UPROPERTY()
	TObjectPtr<UAbilityTask_ApplyRootMotionMoveToForce> DashTask = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageTask = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashDistance = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float FinishVelocityClamp = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	TObjectPtr<UAnimMontage> DashMontage = nullptr;

	UPROPERTY(Transient)
	bool bIsEndingAbility = false;
};
