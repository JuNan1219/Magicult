// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "AbilitySystemComponent.h"
#include "Magicult/Input/MagicultInputComponent.h"



AMagicultPlayerController::AMagicultPlayerController()
{
	bReplicates = true;
}

void AMagicultPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
	CursorDirection();
}

void AMagicultPlayerController::CursorDirection()
{
	if (!CursorHit.bBlockingHit) return;

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		const FVector& PlayerLocation = ControlledPawn->GetActorLocation();
		const FVector& CursorLocation = CursorHit.ImpactPoint;

		FVector Direction = CursorLocation - PlayerLocation;
		Direction.Z = 0.f;

		ControlledPawn->SetActorRelativeRotation(Direction.Rotation());
	}
}


void AMagicultPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;


}

void AMagicultPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *InputTag.ToString());
}

void AMagicultPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Blue, *InputTag.ToString());
}

void AMagicultPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	GEngine->AddOnScreenDebugMessage(3, 3.f, FColor::Green, *InputTag.ToString());
}


void AMagicultPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(MagicultContext);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(MagicultContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AMagicultPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UMagicultInputComponent* MagicultInputComponent = CastChecked<UMagicultInputComponent>(InputComponent);
	MagicultInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMagicultPlayerController::Move);
	MagicultInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AMagicultPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RIghtDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RIghtDirection, InputAxisVector.X);
	}
}
