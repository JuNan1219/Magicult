// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

	// BP에서 오버라이드하여 로비 UI 갱신 트리거
	UFUNCTION(BlueprintNativeEvent, Category = "Lobby")
	void OnLobbyPlayerListChanged();

	virtual void OnLobbyPlayerListChanged_Implementation();

private:
	UFUNCTION()
	void OnStartSessionComplete(bool bWasSuccessful);
};
