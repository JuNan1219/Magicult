// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultWidgetController.h"

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
