// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicultLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

void AMagicultLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMagicultLobbyPlayerState, bIsReady);
}

void AMagicultLobbyPlayerState::ServerSetReady_Implementation(bool bReady)
{
	bIsReady = bReady;
	OnReadyStateChanged.Broadcast();
}

void AMagicultLobbyPlayerState::OnRep_bIsReady()
{
	OnReadyStateChanged.Broadcast();
}
