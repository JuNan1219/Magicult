// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultGameplayAbility.h"
#include "Magicult/MagicultAbilityTypes.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/Attack/AttackDefinition.h"
#include "MagicultDamageGameplayAbility.generated.h"

class UAttackInstance;
class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_PlayMontageAndWait;
struct FGameplayEventData;

/**
 * 공격용 GameplayAbility
 * - GA 생명주기 = 몽타주 생명주기 (몽타주 없으면 AbilityTimeout)
 * - AttackInstance는 판정 전담, GA 종료를 직접 트리거하지 않음
 * - Notify_Attack_End → AttackInstance.Finish() (판정 종료), GA/몽타주는 계속
 */
UCLASS()
class MAGICULT_API UMagicultDamageGameplayAbility : public UMagicultGameplayAbility
{
	GENERATED_BODY()

public:
	UMagicultDamageGameplayAbility();
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
	UFUNCTION()
	void HandleMontageGameplayEvent(FGameplayEventData Payload);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleAbilityTimeout();

	FAttackDefinition GetRandomAttackDefinitionFromArray(const TArray<FAttackDefinition>& AttackDefinitions) const;

private:
	/** 모든 종료 경로의 단일 진입점. 재진입 방어 + AttackInstance 정리 + EndAbility 호출 */
	void TerminateAbility(bool bCancelled);

	void CleanupAttackInstance();

protected:
	UPROPERTY()
	FAttackDefinition CurrentAttackDefinition;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAttackInstance> CurrentAttackInstance = nullptr;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitGameplayEventTask = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageAndWaitTask = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage")
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;

	/** 몽타주 없을 때 GA 최대 지속 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage")
	float AbilityTimeout = 1.f;

	UPROPERTY(Transient)
	bool bIsEndingAbility = false;

	FTimerHandle AbilityTimeoutHandle;
};
