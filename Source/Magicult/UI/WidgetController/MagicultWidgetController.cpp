// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultWidgetController.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "Magicult/Player/MagicultPlayerController.h"
#include "Magicult/Player/MagicultPlayerState.h"
#include "Magicult/Data/MagicInfo.h"



void UMagicultWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;


}

void UMagicultWidgetController::BroadcastInitialValues()
{
	
}

void UMagicultWidgetController::BindCallbacksToDependencies()
{
}

void UMagicultWidgetController::BroadcastMagicInfo()
{
	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
		{
			FMagicultMagicInfo Info = MagicInfo->FindMagicInfoForAbilityTag(MagicultAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));

			//Info.InputTag = MagicultAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
			Info.StatusTag = MagicultAbilitySystemComponent->GetStatusFromSpec(AbilitySpec);
			MagicInfoDelegate.Broadcast(Info);
		});
	GetMagicultASC()->ForEachAbility(BroadcastDelegate);
}

AMagicultPlayerController* UMagicultWidgetController::GetMagicultPC()
{
	if (MagicultPlayerController == nullptr)
	{
		MagicultPlayerController = Cast<AMagicultPlayerController>(PlayerController);
	}
	return MagicultPlayerController;
}

AMagicultPlayerState* UMagicultWidgetController::GetMagicultPS()
{
	if (MagicultPlayerState == nullptr)
	{
		MagicultPlayerState = Cast<AMagicultPlayerState>(PlayerState);
	}
	return MagicultPlayerState;
}

UMagicultAbilitySystemComponent* UMagicultWidgetController::GetMagicultASC()
{
	if (MagicultAbilitySystemComponent == nullptr)
	{
		MagicultAbilitySystemComponent = Cast<UMagicultAbilitySystemComponent>(AbilitySystemComponent);
	}
	return MagicultAbilitySystemComponent;
}

UMagicultAttributeSet* UMagicultWidgetController::GetMagicultAS()
{
	if (MagicultAttributeSet == nullptr)
	{
		MagicultAttributeSet = Cast<UMagicultAttributeSet>(AttributeSet);
	}
	return MagicultAttributeSet;
}
