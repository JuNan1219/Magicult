// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultGameplayTags.h"
#include "GameplayTagsManager.h"

FMagicultGameplayTags FMagicultGameplayTags::GameplayTags;

void FMagicultGameplayTags::InitializeNativeGameplayTags()
{

	// Affinity Attributes
	GameplayTags.Attributes_Affinity_RockAffinity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Affinity.RockAffinity"), 
		FString("")
	);
	GameplayTags.Attributes_Affinity_FlameAffinity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Affinity.FlameAffinity"), 
		FString("")
	);
	GameplayTags.Attributes_Affinity_IceAffinity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Affinity.IceAffinity"), 
		FString("")
	);
	GameplayTags.Attributes_Affinity_WindAffinity = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Affinity.WindAffinity"), 
		FString("")
	);

	// Resistance Attributes
	GameplayTags.Attributes_Resistance_RockResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.RockResistance"), 
		FString("")
	);
	GameplayTags.Attributes_Resistance_FlameResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.FlameResistance"), 
		FString("Reduces the time of flame condition")
	);
	GameplayTags.Attributes_Resistance_IceResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.IceResistance"), 
		FString("")
	);
	GameplayTags.Attributes_Resistance_WindResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Resistance.WindResistance"), 
		FString("")
	);

	// Vital Attributes
	GameplayTags.Attributes_Vital_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.Health"),
		FString("")
	);
	GameplayTags.Attributes_Vital_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxHealth"),
		FString("")
	);
	GameplayTags.Attributes_Vital_Mana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.Mana"),
		FString("")
	);
	GameplayTags.Attributes_Vital_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxMana"),
		FString("")
	);


	// Input Tags
	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"),
		TEXT("마우스 왼쪽 버튼")
	);

	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"),
		TEXT("마우스 오른쪽 버튼")
	);

	GameplayTags.InputTag_0 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.0"),
		TEXT("키보드 숫자 0")
	);

	GameplayTags.InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.1"),
		TEXT("키보드 숫자 1")
	);

	GameplayTags.InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.2"),
		TEXT("키보드 숫자 1")
	);

	GameplayTags.InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.3"),
		TEXT("키보드 숫자 1")
	);

	GameplayTags.InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.4"),
		TEXT("키보드 숫자 1")
	);

	GameplayTags.InputTag_5 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.5"),
		TEXT("키보드 숫자 1")
	);

}
