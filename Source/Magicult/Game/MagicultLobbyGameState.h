// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MagicultLobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPlayerListUpdated);

/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnLobbyPlayerListUpdated OnLobbyPlayerListUpdated;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnPlayerListChanged();
};
