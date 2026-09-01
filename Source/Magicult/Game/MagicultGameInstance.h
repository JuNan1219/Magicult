// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/EngineBaseTypes.h"
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
	virtual void Init() override;
	virtual void Shutdown() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	TObjectPtr<UImmunityPolicy> ImmunityPolicy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	TObjectPtr<UInterruptPolicy> InterruptPolicy;

	// 로비에서 게임플레이 맵으로 ServerTravel하기 직전에 기록하는 기대 접속 인원수.
	// 게임플레이 GameMode의 매치 시작 게이트(ReadyToStartMatch)가 분모로 사용한다.
	// ServerTravel을 넘어 지속되어야 하므로 트래블에서 살아남는 GameInstance에 둔다.
	UPROPERTY(BlueprintReadWrite, Category = "Match")
	int32 ExpectedPlayerCount = 0;

	// 호스트(서버장) 프로세스가 종료/크래시되어 연결이 끊겼을 때 클라이언트가 이동할 메뉴 맵.
	UPROPERTY(EditDefaultsOnly, Category = "Network")
	FString MainMenuMap = TEXT("/Game/Level/MainMenuMap");

private:
	// 호스트 이탈 등으로 네트워크 연결이 끊겼을 때: 클라이언트를 튕기지 않고 메인 메뉴로 복귀시킨다.
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

	void TravelToMainMenu();

	// 실패 연쇄로 여러 번 트래블하는 것을 막는 가드.
	bool bReturningToMainMenu = false;
};
