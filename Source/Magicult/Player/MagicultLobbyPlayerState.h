// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MagicultLobbyPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReadyStateChanged);

UCLASS()
class MAGICULT_API AMagicultLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable)
	FOnReadyStateChanged OnReadyStateChanged;

	UPROPERTY(ReplicatedUsing = OnRep_bIsReady, BlueprintReadOnly)
	bool bIsReady = false;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerSetReady(bool bReady);

private:
	UFUNCTION()
	void OnRep_bIsReady();
};
