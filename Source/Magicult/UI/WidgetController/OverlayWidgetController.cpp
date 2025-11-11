// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlayWidgetController.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const UMagicultAttributeSet* AuraAttributeSet = CastChecked<UMagicultAttributeSet>(AttributeSet);

	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
}
