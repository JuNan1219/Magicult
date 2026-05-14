// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "MagicultAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicultAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
