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
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, ComputationalPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, MaxComputationalPower, COND_None, REPNOTIFY_Always);
}

void UMagicultAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, Health, OldHealth);
}

void UMagicultAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, MaxHealth, OldMaxHealth);
}

void UMagicultAttributeSet::OnRep_ComputationalPower(const FGameplayAttributeData& OldComputationalPower) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, ComputationalPower, OldComputationalPower);
}

void UMagicultAttributeSet::OnRep_MaxComputationalPower(const FGameplayAttributeData& OldMaxComputationalPower) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, MaxComputationalPower, OldMaxComputationalPower);
}
