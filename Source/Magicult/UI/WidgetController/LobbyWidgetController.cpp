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
		if (!MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsAlreadyBound(this, &ThisClass::OnLeaveComplete))
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnLeaveComplete);
		}
		MultiplayerSessionsSubsystem->DestroySession();
	}
	else
	{
		PlayerController->ClientTravel("/Game/Level/MainMenuMap", TRAVEL_Absolute);
	}
}

void ULobbyWidgetController::OnLeaveComplete(bool bWasSuccessful)
{
	// GameInstanceSubsystem은 레벨 전환 후에도 살아있으므로 바인딩을 직접 제거해야 누적 호출을 막을 수 있음
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &ThisClass::OnLeaveComplete);
	}
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

	if (!LobbyGameState) return MagicultLobbyPlayers;

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
