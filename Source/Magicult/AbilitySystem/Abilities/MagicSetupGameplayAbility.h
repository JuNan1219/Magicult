// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultGameplayAbility.h"
#include "MagicSetupGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicSetupGameplayAbility : public UMagicultGameplayAbility
{
	GENERATED_BODY()

public:
	UMagicSetupGameplayAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
