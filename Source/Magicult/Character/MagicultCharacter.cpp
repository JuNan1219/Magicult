// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Magicult/Player/MagicultPlayerState.h"
#include "Magicult/UI/HUD/MagicultHUD.h"


AMagicultCharacter::AMagicultCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AMagicultCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMagicultCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 서버 : Init ability actor info
	InitAbilityActorInfo();
}

void AMagicultCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트 : Init ability actor info
	InitAbilityActorInfo();
}

void AMagicultCharacter::InitAbilityActorInfo()
{
	AMagicultPlayerState* MagicultPlayerState = GetPlayerState<AMagicultPlayerState>();
	check(MagicultPlayerState);
	MagicultPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(MagicultPlayerState, this);
	AbilitySystemComponent = MagicultPlayerState->GetAbilitySystemComponent();
	AttributeSet = MagicultPlayerState->GetAttributeSet();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (AMagicultHUD* HUD = PlayerController->GetHUD<AMagicultHUD>())
		{
			HUD->InitOverlay(PlayerController, MagicultPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	
}
