// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Magicult/Player/MagicultPlayerState.h"
#include "Magicult/UI/HUD/MagicultHUD.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Net/UnrealNetwork.h"

AMagicultCharacter::AMagicultCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void AMagicultCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMagicultCharacter, PlayerIndex);
}

void AMagicultCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMagicultCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// AGameModeBase는 SeamlessTravel 시 OverrideWith를 호출하지 않으므로
	// PlayerState 대신 PlayerArray 순서로 PlayerIndex 결정
	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		if (APlayerState* PS = GetPlayerState<APlayerState>())
		{
			for (int32 i = 0; i < GS->PlayerArray.Num(); ++i)
			{
				if (GS->PlayerArray[i].Get() == PS)
				{
					PlayerIndex = i;
					break;
				}
			}
		}
	}
	ApplyPlayerColor();

	InitAbilityActorInfo();
	AddCharacterAbilities();

	// 속성 초기화(InitAbilityActorInfo→InitializeStartupData) 이후 서버에서 리젠 GE 적용
	ApplyHealthRegenEffect();
}

void AMagicultCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트: Init ability actor info
	InitAbilityActorInfo();
}

void AMagicultCharacter::OnRep_PlayerIndex()
{
	ApplyPlayerColor();
}

void AMagicultCharacter::ApplyPlayerColor()
{
	UMaterialInterface* Material = (PlayerIndex == 0) ? Player1Material : Player2Material;
	if (Material && GetMesh())
	{
		GetMesh()->SetMaterial(0, Material);
	}
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
