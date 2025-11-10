// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultUserWidget.h"

void UMagicultUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
