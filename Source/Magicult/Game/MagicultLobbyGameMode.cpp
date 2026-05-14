// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicultLobbyGameMode.h"
#include "Magicult/Player/MagicultLobbyPlayerState.h"
#include "Magicult/Game/MagicultLobbyGameState.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"

AMagicultLobbyGameMode::AMagicultLobbyGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerStateClass = AMagicultLobbyPlayerState::StaticClass();
	GameStateClass = AMagicultLobbyGameState::StaticClass();

}

void AMagicultLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	OnLobbyPlayerListChanged();
}

void AMagicultLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	OnLobbyPlayerListChanged();

	// 호스트의 플레이어 컨트롤러는 GetFirstPlayerController
	// 호스트가 나갔다면 모든 플레이어 MainMenu로 이동
	if (Exiting == GetWorld()->GetFirstPlayerController())
	{ 
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) 
		{ 
			if (APlayerController* PC = It->Get())
				PC->ClientTravel("/Game/Level/MainMenuMap", TRAVEL_Absolute);
		}
	}
}

void AMagicultLobbyGameMode::StartGame()
{
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMultiplayerSessionsSubsystem* MSS = GI->GetSubsystem<UMultiplayerSessionsSubsystem>())
		{
			MSS->StartSession();
		}
	}

	World->ServerTravel(GameMapPath + TEXT("?listen"));
}

bool AMagicultLobbyGameMode::AreAllPlayersReady() const
{
	if (!GameState) return false;

	const int32 NumPlayers = GameState->PlayerArray.Num();
	if (NumPlayers < MinPlayersToStart) return false;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		const AMagicultLobbyPlayerState* LobbyPS = Cast<AMagicultLobbyPlayerState>(PS);
		if (!LobbyPS || !LobbyPS->bIsReady)
		{
			return false;
		}
	}

	return true;
}

void AMagicultLobbyGameMode::OnLobbyPlayerListChanged_Implementation()
{
	AMagicultLobbyGameState* LobbyGS = GetGameState<AMagicultLobbyGameState>(); 
	LobbyGS -> MulticastOnPlayerListChanged();
}
