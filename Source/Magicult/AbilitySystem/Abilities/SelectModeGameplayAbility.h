// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultGameplayAbility.h"
#include "SelectModeGameplayAbility.generated.h"

class UGameplayEffect;
class UInputToTagConfig;
class UMagicInfo;
class AMagicultPlayerController;

/**
 * 
 */
UCLASS()
class MAGICULT_API USelectModeGameplayAbility : public UMagicultGameplayAbility
{
	GENERATED_BODY()

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UPROPERTY(BlueprintReadOnly)
	FActiveGameplayEffectHandle AppliedEffectHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataAsset")
	TObjectPtr<UInputToTagConfig> InputToTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataAsset")
	TObjectPtr<UMagicInfo> MagicInfomation;

	UPROPERTY()
	TObjectPtr<AMagicultPlayerController> MagicultPlayerController;
};
