// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultHUD.h"

#include "Magicult/UI/Widget/MagicultUserWidget.h"
#include "Magicult/UI/WidgetController/OverlayWidgetController.h"
#include "Magicult/UI/WidgetController/MagicMenuWidgetController.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

UOverlayWidgetController* AMagicultHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();

		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

UMagicMenuWidgetController* AMagicultHUD::GetMagicMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if (MagicMenuWidgetController == nullptr)
	{
		MagicMenuWidgetController = NewObject<UMagicMenuWidgetController>(this, MagicMenuWidgetControllerClass);
		MagicMenuWidgetController->SetWidgetControllerParams(WCParams);
		MagicMenuWidgetController->BindCallbacksToDependencies();
	}
	return MagicMenuWidgetController;
}

void AMagicultHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_MagicultHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_MagicultHUD"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UMagicultUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(OverlayWidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}
