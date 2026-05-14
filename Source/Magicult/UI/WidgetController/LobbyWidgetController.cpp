// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidgetController.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Magicult/Game/MagicultLobbyGameState.h"
#include "Magicult/Player/MagicultLobbyPlayerState.h"

void ULobbyWidgetController::Initialize(APlayerController* PC)
{
	PlayerController = PC;
	if (PlayerController)
	{
		MultiplayerSessionsSubsystem = PlayerController->GetGameInstance()->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}
	LobbyGameState = GetWorld()->GetGameState<AMagicultLobbyGameState>();
	if (LobbyGameState)
	{
		LobbyGameState->OnLobbyPlayerListUpdated.AddDynamic(this, &ULobbyWidgetController::HandlePlayerListUpdated);
	}

	HandlePlayerListUpdated();
}

void ULobbyWidgetController::LeaveSession()
{
	if (IsLocalPlayerHost() && MultiplayerSessionsSubsystem)
	{
		// DestroySession → 완료 델리게이트에서 OpenLevel
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnLeaveComplete);
		MultiplayerSessionsSubsystem->DestroySession();
	}
	else
	{
		PlayerController->ClientTravel("/Game/Level/MainMenuMap", TRAVEL_Absolute);
	}
}

void ULobbyWidgetController::OnLeaveComplete(bool bWasSuccessful)
{
	PlayerController->ClientTravel("/Game/Level/MainMenuMap", TRAVEL_Absolute);
}

void ULobbyWidgetController::HandlePlayerListUpdated()
{
	TArray<AMagicultLobbyPlayerState*> LobbyPlayerStates = GetLobbyPlayers();

	for (auto* PlayerState : LobbyPlayerStates)
	{
		if (!IsValid(PlayerState))
		{
			continue;
		}
		if (!PlayerState->OnReadyStateChanged.IsAlreadyBound(this, &ThisClass::HandleReadyStateChanged))
		{
			PlayerState->OnReadyStateChanged.AddUniqueDynamic(this, &ThisClass::HandleReadyStateChanged);
		}
	}

	OnPlayerListChanged.Broadcast();
}

void ULobbyWidgetController::HandleReadyStateChanged()
{
	OnPlayerListChanged.Broadcast();
}

TArray<AMagicultLobbyPlayerState*> ULobbyWidgetController::GetLobbyPlayers()
{
	TArray<AMagicultLobbyPlayerState*> MagicultLobbyPlayers;

	for (auto& PlayerState : LobbyGameState->PlayerArray)
	{
		if (AMagicultLobbyPlayerState* LobbyPlayerState = Cast<AMagicultLobbyPlayerState>(PlayerState))
			MagicultLobbyPlayers.Add(LobbyPlayerState);
	}

	return MagicultLobbyPlayers;
}

bool ULobbyWidgetController::IsLocalPlayerHost()
{
	return GetWorld()->GetNetMode() != NM_Client;
}
