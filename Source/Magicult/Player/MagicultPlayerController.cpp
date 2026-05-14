// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "Magicult/Input/MagicultInputComponent.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/UI/HUD/MagicultHUD.h"


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

	APawn* ControlledPawn = GetPawn<APawn>();
	if (!ControlledPawn) return;

	FVector Direction = CursorHit.ImpactPoint - ControlledPawn->GetActorLocation();
	Direction.Z = 0.f;
	const float TargetYaw = Direction.Rotation().Yaw;

	// 로컬 즉시 반영 (응답성 유지)
	ControlledPawn->SetActorRelativeRotation(FRotator(0.f, TargetYaw, 0.f));

	// 리슨 서버는 이미 Authority이므로 RPC 불필요
	if (HasAuthority()) return;

	// 임계값 이상 변했을 때만 전송 (매 프레임 RPC 방지)
	if (FMath::Abs(FRotator::NormalizeAxis(TargetYaw - LastSentYaw)) > 1.f)
	{
		LastSentYaw = TargetYaw;
		ServerSetFacingYaw(TargetYaw);
	}
}

void AMagicultPlayerController::ServerSetFacingYaw_Implementation(float Yaw)
{
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->SetActorRotation(FRotator(0.f, Yaw, 0.f));
	}
}

void AMagicultPlayerController::AddMagicDetailInputContext()
{
	check(MagicDetailContext);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		FModifyContextOptions ModifyContextOptions;
		ModifyContextOptions.bIgnoreAllPressedKeysUntilRelease = true;
		Subsystem->AddMappingContext(MagicDetailContext, 1, ModifyContextOptions);
	}
}

void AMagicultPlayerController::RemoveMagicDetailInputContext()
{
	check(MagicDetailContext);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->RemoveMappingContext(MagicDetailContext);
	}
}

void AMagicultPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;
}

void AMagicultPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagPressed(InputTag);
}

void AMagicultPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagReleased(InputTag);
}

void AMagicultPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() == nullptr) return;
	GetASC()->AbilityInputTagHeld(InputTag);
}

UMagicultAbilitySystemComponent* AMagicultPlayerController::GetASC()
{
	if (!MagicultAbilitySystemComponent.IsValid())
	{
		MagicultAbilitySystemComponent = Cast<UMagicultAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return MagicultAbilitySystemComponent.Get();
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
	MagicultInputComponent->BindAction(RotationAction, ETriggerEvent::Triggered, this, &AMagicultPlayerController::Rotation);
	MagicultInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);

	MagicultInputComponent->BindAction(MenuAction, ETriggerEvent::Started, this, &AMagicultPlayerController::HandleMenuInput);
}

void AMagicultPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AMagicultPlayerController::Rotation(const FInputActionValue& InputActionValue)
{
	const float AxisValue = InputActionValue.Get<float>();
	if (FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	FRotator ControlRot = GetControlRotation();

	// 1초에 45도 회전
	const float DeltaYaw = AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds();
	ControlRot.Yaw = FRotator::NormalizeAxis(ControlRot.Yaw + DeltaYaw);

	SetControlRotation(ControlRot);
}

void AMagicultPlayerController::HandleMenuInput(const FInputActionValue& InputActionValue)
{
	bool bIsPressed = InputActionValue.Get<bool>();

	if (bIsPressed)
	{
		if (AMagicultHUD* HUD = Cast<AMagicultHUD>(GetHUD()))
		{
			HUD->HandleMagicMenu();
		}
	}
}