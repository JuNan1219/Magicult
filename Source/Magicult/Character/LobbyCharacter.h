// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LobbyCharacter.generated.h"

UCLASS()
class MAGICULT_API ALobbyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALobbyCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 서버에서만 호출 — PostLogin 시 GameMode가 설정
	void SetPlayerIndex(int32 InIndex);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PlayerIndex)
	int32 PlayerIndex = -1;

protected:
	virtual void BeginPlay() override;

	// 인덱스 0 = 1P 파란색, 인덱스 1 = 2P 빨간색
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Color")
	TObjectPtr<UMaterialInterface> Player1Material;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|Color")
	TObjectPtr<UMaterialInterface> Player2Material;

private:
	UFUNCTION()
	void OnRep_PlayerIndex();

	void ApplyPlayerColor();
};
