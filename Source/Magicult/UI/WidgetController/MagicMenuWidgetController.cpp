// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicMenuWidgetController.h"

#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/Data/MagicInfo.h"
#include "Magicult/MagicultGameplayTags.h"

void UMagicMenuWidgetController::BroadcastInitialValues()
{
	check(MagicInfo);

	for (auto& Info : MagicInfo->MagicInformation)
	{
		//Info.InputTag = FMagicultGameplayTags::Get().Magic_Type_None;
		Info.StatusTag = FMagicultGameplayTags::Get().Magic_Status_Locked;
		MagicInfoDelegate.Broadcast(Info);
	}

	BroadcastMagicInfo();
}

void UMagicMenuWidgetController::BindCallbacksToDependencies()
{
	GetMagicultASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
		{
			if (MagicInfo)
			{
				FMagicultMagicInfo Info = MagicInfo->FindMagicInfoForAbilityTag(AbilityTag);
				Info.StatusTag = StatusTag;
				MagicInfoDelegate.Broadcast(Info);
			}
		});
}