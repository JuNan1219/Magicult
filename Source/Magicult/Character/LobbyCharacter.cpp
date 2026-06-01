// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

ALobbyCharacter::ALobbyCharacter()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->DisableMovement();
}

void ALobbyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서는 SetPlayerIndex로 이미 적용됨
	// 클라이언트는 OnRep_PlayerIndex가 처리하지만, 만약 이미 복제된 값이면 여기서 적용
	if (PlayerIndex >= 0)
	{
		ApplyPlayerColor();
	}
}

void ALobbyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyCharacter, PlayerIndex);
}

void ALobbyCharacter::SetPlayerIndex(int32 InIndex)
{
	PlayerIndex = InIndex;
	ApplyPlayerColor();
}

void ALobbyCharacter::OnRep_PlayerIndex()
{
	ApplyPlayerColor();
}

void ALobbyCharacter::ApplyPlayerColor()
{
	UMaterialInterface* Material = (PlayerIndex == 0) ? Player1Material : Player2Material;
	if (Material && GetMesh())
	{
		GetMesh()->SetMaterial(0, Material);
	}
}
