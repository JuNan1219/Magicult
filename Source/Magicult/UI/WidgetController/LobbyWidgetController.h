// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LobbyWidgetController.generated.h"

class AMagicultLobbyGameState;
class APlayerController;
class AMagicultLobbyPlayerState;
class UMultiplayerSessionsSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPlayersUpdated);

/**
 * 
 */
UCLASS(BlueprintType)
class MAGICULT_API ULobbyWidgetController : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnLobbyPlayersUpdated OnPlayerListChanged;

	UFUNCTION(BlueprintCallable)
	void Initialize(APlayerController* PC);

	UFUNCTION(BlueprintCallable)
	TArray<AMagicultLobbyPlayerState*> GetLobbyPlayers();

	UFUNCTION(BlueprintCallable)
	bool IsLocalPlayerHost();

	UFUNCTION(BlueprintCallable)
	void LeaveSession();

	UFUNCTION() 
	void OnLeaveComplete(bool bWasSuccessful);

private:
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<AMagicultLobbyGameState> LobbyGameState;

	UPROPERTY()
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

	UFUNCTION()
	void HandlePlayerListUpdated();

	UFUNCTION()
	void HandleReadyStateChanged();
};
