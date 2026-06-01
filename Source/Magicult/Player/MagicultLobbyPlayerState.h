// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MagicultLobbyPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReadyStateChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerIndexChanged);

UCLASS()
class MAGICULT_API AMagicultLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMagicultLobbyPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable)
	FOnReadyStateChanged OnReadyStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerIndexChanged OnPlayerIndexChanged;

	UPROPERTY(ReplicatedUsing = OnRep_bIsReady, BlueprintReadOnly)
	bool bIsReady = false;

	// 0 = 1P(파란색), 1 = 2P(빨간색) — 서버에서 PostLogin 시 할당
	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex, BlueprintReadOnly)
	int32 PlayerIndex = -1;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerSetReady(bool bReady);

	// 서버 전용 (GameMode에서 직접 호출)
	void SetPlayerIndex(int32 InIndex);

private:
	UFUNCTION()
	void OnRep_bIsReady();

	UFUNCTION()
	void OnRep_PlayerIndex();
};
