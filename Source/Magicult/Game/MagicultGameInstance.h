// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MagicultGameInstance.generated.h"

class UImmunityPolicy;
class UInterruptPolicy;

/**
 *
 */
UCLASS()
class MAGICULT_API UMagicultGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	TObjectPtr<UImmunityPolicy> ImmunityPolicy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	TObjectPtr<UInterruptPolicy> InterruptPolicy;

	// 로비에서 게임플레이 맵으로 ServerTravel하기 직전에 기록하는 기대 접속 인원수.
	// 게임플레이 GameMode의 매치 시작 게이트(ReadyToStartMatch)가 분모로 사용한다.
	// ServerTravel을 넘어 지속되어야 하므로 트래블에서 살아남는 GameInstance에 둔다.
	UPROPERTY(BlueprintReadWrite, Category = "Match")
	int32 ExpectedPlayerCount = 0;
};
