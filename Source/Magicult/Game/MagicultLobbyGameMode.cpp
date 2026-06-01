// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicultLobbyGameMode.h"
#include "Magicult/Player/MagicultLobbyPlayerState.h"
#include "Magicult/Game/MagicultLobbyGameState.h"
#include "Magicult/Character/LobbyCharacter.h"

#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

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

	// PlayerIndex 할당 (PlayerArray에서 현재 위치)
	int32 NewPlayerIndex = -1;
	if (AMagicultLobbyPlayerState* LPS = NewPlayer->GetPlayerState<AMagicultLobbyPlayerState>())
	{
		for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
		{
			if (GameState->PlayerArray[i].Get() == LPS)
			{
				NewPlayerIndex = i;
				break;
			}
		}
		LPS->SetPlayerIndex(NewPlayerIndex);
	}

	SpawnLobbyCharacterFor(NewPlayer, NewPlayerIndex);
	OnLobbyPlayerListChanged();
}

void AMagicultLobbyGameMode::SpawnLobbyCharacterFor(APlayerController* PC, int32 InPlayerIndex)
{
	if (!PC || !LobbyCharacterClass) return;

	AActor* SpawnPoint = FindLobbySpawnPoint(InPlayerIndex);
	const FTransform SpawnTransform = SpawnPoint ? SpawnPoint->GetActorTransform() : FTransform::Identity;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ALobbyCharacter* Character = GetWorld()->SpawnActor<ALobbyCharacter>(LobbyCharacterClass, SpawnTransform, Params);
	if (Character)
	{
		PC->Possess(Character);
		Character->SetPlayerIndex(InPlayerIndex);
	}
}

AActor* AMagicultLobbyGameMode::FindLobbySpawnPoint(int32 InPlayerIndex) const
{
	// PlayerStart에 태그 "P0", "P1" 등을 설정하면 해당 위치에 스폰
	const FName TargetTag = FName(*FString::Printf(TEXT("P%d"), InPlayerIndex));
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == TargetTag)
		{
			return *It;
		}
	}
	// 태그 없으면 처음 발견된 PlayerStart 사용
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void AMagicultLobbyGameMode::Logout(AController* Exiting)
{
	// 로비 캐릭터 제거
	if (APawn* Pawn = Exiting->GetPawn())
	{
		Pawn->Destroy();
	}

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
