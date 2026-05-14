// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultLobbyGameState.h"

void AMagicultLobbyGameState::MulticastOnPlayerListChanged_Implementation()
{
	OnLobbyPlayerListUpdated.Broadcast();
}
