// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultHUD.h"

#include "Magicult/UI/Widget/MagicultUserWidget.h"

void AMagicultHUD::BeginPlay()
{
	Super::BeginPlay();

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	Widget->AddToViewport();

}
