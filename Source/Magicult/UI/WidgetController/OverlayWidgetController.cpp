// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlayWidgetController.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "GameplayEffectTypes.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const UMagicultAttributeSet* MagicultAttributeSet = CastChecked<UMagicultAttributeSet>(AttributeSet);

	OnHealthChanged.Broadcast(MagicultAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(MagicultAttributeSet->GetMaxHealth());
	OnMaxManaChanged.Broadcast(MagicultAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(MagicultAttributeSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependecies()
{
	const UMagicultAttributeSet* MagicultAttributeSet = CastChecked<UMagicultAttributeSet>(AttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MagicultAttributeSet->GetHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MagicultAttributeSet->GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::MaxHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MagicultAttributeSet->GetManaAttribute()).AddUObject(this, &UOverlayWidgetController::ManaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MagicultAttributeSet->GetMaxManaAttribute()).AddUObject(this, &UOverlayWidgetController::MaxManaChanged);
}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& Data) const
{
	OnManaChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxManaChanged.Broadcast(Data.NewValue);
}
