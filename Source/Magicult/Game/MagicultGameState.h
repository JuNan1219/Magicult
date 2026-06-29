// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MagicultGameState.generated.h"

class UUserWidget;

/**
 * 게임플레이 맵용 GameState.
 * AGameState를 상속하여 MatchState 복제를 활성화한다(매치 시작 게이트가 의존).
 * 추가로, 매치 시작 전(WaitingToStart) 동안 플레이어 폰이 아직 스폰되지 않아 보이는
 * "빈 월드"를 가리기 위해, MatchState에 연동된 대기 위젯을 로컬 클라이언트에 표시/제거한다.
 */
UCLASS()
class MAGICULT_API AMagicultGameState : public AGameState
{
	GENERATED_BODY()

public:
	// WaitingToStart 동안 뷰포트를 덮을 대기 위젯 클래스 (BP에서 지정).
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	TSubclassOf<UUserWidget> WaitingWidgetClass;

protected:
	virtual void BeginPlay() override;

	// AGameState가 OnRep_MatchState에서 클라이언트에서도 호출하는 핸들러들.
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void HandleMatchHasStarted() override;

private:
	UPROPERTY()
	TObjectPtr<UUserWidget> WaitingWidget;

	FTimerHandle RetryTimerHandle;

	void ShowWaitingWidget();
	void HideWaitingWidget();
};
