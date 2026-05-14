// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Magicult/Player/MagicultPlayerState.h"
#include "Magicult/UI/HUD/MagicultHUD.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/MagicultGameplayTags.h"

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
	AddCharacterAbilities();
}

void AMagicultCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트 : Init ability actor info
	InitAbilityActorInfo();
}

int32 AMagicultCharacter::GetPlayerLevel_Implementation()
{
	const AMagicultPlayerState* MagicultPlayerState = GetPlayerState<AMagicultPlayerState>();
	check(MagicultPlayerState);
	return MagicultPlayerState->GetPlayerLevel();
}

FName AMagicultCharacter::GetDefaultKnockbackBone() const
{
	return TEXT("Head_01");
}

void AMagicultCharacter::Die()
{
	Super::Die();

	/*FTimerDelegate DeathTimerDelegate;
	DeathTimerDelegate.BindLambda([this]()
		{
			AMagicultGameModeBase* MagicultGM = Cast<AMagicultGameModeBase>(UGameplayStatics::GetGameMode(this));
			if (MagicultGM)
			{
				MagicultGM->PlayerDied(this);
			}
		});*/
}

void AMagicultCharacter::InitAbilityActorInfo()
{
	AMagicultPlayerState* MagicultPlayerState = GetPlayerState<AMagicultPlayerState>();
	check(MagicultPlayerState);
	MagicultPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(MagicultPlayerState, this);
	Cast<UMagicultAbilitySystemComponent>(MagicultPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = MagicultPlayerState->GetAbilitySystemComponent();
	AttributeSet = MagicultPlayerState->GetAttributeSet();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (AMagicultHUD* HUD = PlayerController->GetHUD<AMagicultHUD>())
		{
			HUD->InitOverlay(PlayerController, MagicultPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	InitializeDefaultAttributes();

	Super::InitAbilityActorInfo();
}

void AMagicultCharacter::InitializeDefaultAttributes() const
{
	if (AMagicultPlayerState* MagicultPlayerState = GetPlayerState<AMagicultPlayerState>())
	{
		MagicultPlayerState->InitializeStartupData();
	}
}
