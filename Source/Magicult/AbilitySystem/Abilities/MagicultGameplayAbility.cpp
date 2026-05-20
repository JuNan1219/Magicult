// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultGameplayAbility.h"
#include "AbilitySystemComponent.h"

void UMagicultGameplayAbility::CancelAbilitiesWithTag(FGameplayTag Tag)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayTagContainer Tags(Tag);
		ASC->CancelAbilities(&Tags);
	}
}
