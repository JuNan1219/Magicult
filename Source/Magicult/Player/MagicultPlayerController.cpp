// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultPlayerController.h"

#include "EnhancedInputSubsystems.h"

AMagicultPlayerController::AMagicultPlayerController()
{
	bReplicates = true;
}

void AMagicultPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(MagicultContext);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);

	Subsystem->AddMappingContext(MagicultContext, 0);

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}
