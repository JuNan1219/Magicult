// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * MagiucultGameplayTags
 * 
 * Singleton contating natice Gameplay Tags
 */

struct FMagicultGameplayTags
{
public:
	static const FMagicultGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();


	FGameplayTag Attributes_Affinity_RockAffinity;
	FGameplayTag Attributes_Affinity_FlameAffinity;
	FGameplayTag Attributes_Affinity_IceAffinity;
	FGameplayTag Attributes_Affinity_WindAffinity;

	FGameplayTag Attributes_Resistance_RockResistance;
	FGameplayTag Attributes_Resistance_FlameResistance;
	FGameplayTag Attributes_Resistance_IceResistance;
	FGameplayTag Attributes_Resistance_WindResistance;

	FGameplayTag Attributes_Vital_Health;
	FGameplayTag Attributes_Vital_MaxHealth;
	FGameplayTag Attributes_Vital_Mana;
	FGameplayTag Attributes_Vital_MaxMana;

	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_0;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_5;
protected:

private:
	static FMagicultGameplayTags GameplayTags;
};