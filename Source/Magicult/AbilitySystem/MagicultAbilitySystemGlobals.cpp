// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultAbilitySystemGlobals.h"
#include "Magicult/MagicultAbilityTypes.h"

FGameplayEffectContext* UMagicultAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FMagicultGameplayEffectContext();
}
