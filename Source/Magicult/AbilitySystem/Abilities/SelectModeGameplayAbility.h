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

public:
	USelectModeGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 이 인스턴스가 자기 ASC에 붙여둔 모드 태그. 유효하면 "내가 붙였다"는 뜻이고,
	// EndAbility에서 반드시 같은 태그를 빼야 한다(loose 태그는 참조 카운트다).
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag AppliedModeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataAsset")
	TObjectPtr<UInputToTagConfig> InputToTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataAsset")
	TObjectPtr<UMagicInfo> MagicInfomation;

	UPROPERTY()
	TObjectPtr<AMagicultPlayerController> MagicultPlayerController;
};
