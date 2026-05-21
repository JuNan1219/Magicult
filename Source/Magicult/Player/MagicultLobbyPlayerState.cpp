// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicultLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

AMagicultLobbyPlayerState::AMagicultLobbyPlayerState()
{
	// APlayerState 기본값이 1Hz라 레디 상태가 최대 1초 뒤에 반영됨
	NetUpdateFrequency = 100.f;
}

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
