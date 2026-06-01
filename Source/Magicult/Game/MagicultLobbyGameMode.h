// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Magicult/Character/LobbyCharacter.h"
#include "MagicultLobbyGameMode.generated.h"

UCLASS()
class MAGICULT_API AMagicultLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMagicultLobbyGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable)
	void StartGame();

	UFUNCTION(BlueprintPure)
	bool AreAllPlayersReady() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	FString GameMapPath = TEXT("/Game/Level/TestMap");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	int32 MinPlayersToStart = 1;

	// 로비에서 스폰할 캐릭터 클래스 (BP에서 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Character")
	TSubclassOf<ALobbyCharacter> LobbyCharacterClass;

	// BP에서 오버라이드하여 로비 UI 갱신 트리거
	UFUNCTION(BlueprintNativeEvent, Category = "Lobby")
	void OnLobbyPlayerListChanged();

	virtual void OnLobbyPlayerListChanged_Implementation();

private:
	void SpawnLobbyCharacterFor(APlayerController* PC, int32 InPlayerIndex);
	AActor* FindLobbySpawnPoint(int32 InPlayerIndex) const;

	UFUNCTION()
	void OnStartSessionComplete(bool bWasSuccessful);
};
