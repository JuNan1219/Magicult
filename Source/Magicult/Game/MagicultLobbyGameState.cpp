// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultLobbyGameState.h"

void AMagicultLobbyGameState::MulticastOnPlayerListChanged_Implementation()
{
	OnLobbyPlayerListUpdated.Broadcast();
}

void AMagicultLobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	OnLobbyPlayerListUpdated.Broadcast();
}

void AMagicultLobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	OnLobbyPlayerListUpdated.Broadcast();
}
