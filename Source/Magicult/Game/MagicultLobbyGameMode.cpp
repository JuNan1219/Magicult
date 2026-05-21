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

	bUseSeamlessTravel = true;
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

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMultiplayerSessionsSubsystem* MSS = GI->GetSubsystem<UMultiplayerSessionsSubsystem>())
		{
			MSS->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSessionComplete);
			MSS->StartSession();
		}
	}
}

void AMagicultLobbyGameMode::OnStartSessionComplete(bool bWasSuccessful)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMultiplayerSessionsSubsystem* MSS = GI->GetSubsystem<UMultiplayerSessionsSubsystem>())
		{
			MSS->MultiplayerOnStartSessionComplete.RemoveDynamic(this, &ThisClass::OnStartSessionComplete);
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(GameMapPath + TEXT("?listen"));
	}
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
	// AddPlayerState/RemovePlayerState 오버라이드에서 클라이언트 UI 갱신을 처리하므로
	// Multicast RPC 불필요 — 서버 측 GameState 직접 브로드캐스트만 수행
	AMagicultLobbyGameState* LobbyGS = GetGameState<AMagicultLobbyGameState>();
	if (LobbyGS)
	{
		LobbyGS->OnLobbyPlayerListUpdated.Broadcast();
	}
}
