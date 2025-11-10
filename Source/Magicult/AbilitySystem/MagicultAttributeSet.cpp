// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultAttributeSet.h"

#include "Net/UnrealNetwork.h"

UMagicultAttributeSet::UMagicultAttributeSet()
{
	InitHealth(100.f);
}

void UMagicultAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
}

void UMagicultAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, Health, OldHealth);
}

void UMagicultAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, MaxHealth, OldMaxHealth);
}

void UMagicultAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, Mana, OldMana);
}

void UMagicultAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, MaxMana, OldMaxMana);
}
