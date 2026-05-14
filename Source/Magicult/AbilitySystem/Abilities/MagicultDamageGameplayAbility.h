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
 * - ActivateAbility 시 AttackInstance를 생성/초기화하고 즉시 Execute
 * - Montage Notify -> GameplayEvent를 WaitGameplayEvent로 받아 AttackInstance에 전달
 * - Montage가 중단되면 AttackInstance를 취소하고 Ability 종료
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

	void HandleAttackInstanceFinished(UAttackInstance* FinishedInstance);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	UFUNCTION()
	void HandleMontageCancelled();

	FAttackDefinition GetRandomAttackDefinitionFromArray(const TArray<FAttackDefinition>& AttackDefinitions) const;
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

	UPROPERTY(Transient)
	bool bIsEndingAbility = false;
};