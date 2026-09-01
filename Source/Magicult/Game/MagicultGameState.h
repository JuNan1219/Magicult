// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MagicultGameState.generated.h"

class UUserWidget;

/**
 * 매치 결과. MatchState는 WaitingPostMatch까지만 알려주고 승/패를 구분하지 못하므로,
 * 결과는 별도 복제 프로퍼티로 내려보낸다(중간 참가자도 OnRep 없이 현재 값을 받는다).
 */
UENUM(BlueprintType)
enum class EMatchOutcome : uint8
{
	None,
	Victory,
	Defeat
};

/**
 * 게임플레이 맵용 GameState.
 * AGameState를 상속하여 MatchState 복제를 활성화한다(매치 시작 게이트가 의존).
 * 추가로, 매치 시작 전(WaitingToStart) 동안 플레이어 폰이 아직 스폰되지 않아 보이는
 * "빈 월드"를 가리기 위해, MatchState에 연동된 대기 위젯을 로컬 클라이언트에 표시/제거하고,
 * 매치가 끝나면 MatchOutcome에 맞는 결과 위젯을 띄운다.
 */
UCLASS()
class MAGICULT_API AMagicultGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// WaitingToStart 동안 뷰포트를 덮을 대기 위젯 클래스 (BP에서 지정).
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	TSubclassOf<UUserWidget> WaitingWidgetClass;

	// 매치 종료 시 결과에 따라 띄울 위젯 (BP에서 지정). 미지정이면 아무것도 뜨지 않는다.
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	TSubclassOf<UUserWidget> DefeatWidgetClass;

	// 매치 결과를 확정한다(서버 전용). 리슨 서버는 OnRep이 돌지 않으므로 직접 표시까지 처리한다.
	void SetMatchOutcome(EMatchOutcome NewOutcome);

	UFUNCTION(BlueprintPure, Category = "Match")
	EMatchOutcome GetMatchOutcome() const { return MatchOutcome; }

protected:
	virtual void BeginPlay() override;

	// AGameState가 OnRep_MatchState에서 클라이언트에서도 호출하는 핸들러들.
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void HandleMatchHasStarted() override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_MatchOutcome)
	EMatchOutcome MatchOutcome = EMatchOutcome::None;

	UFUNCTION()
	void OnRep_MatchOutcome();

	UPROPERTY()
	TObjectPtr<UUserWidget> WaitingWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> ResultWidget;

	FTimerHandle RetryTimerHandle;
	FTimerHandle ResultRetryTimerHandle;

	void ShowWaitingWidget();
	void HideWaitingWidget();
	void ShowResultWidget();

	// 로컬 플레이어 컨트롤러가 아직 없으면 nullptr을 돌려준다(호출부가 재시도를 건다).
	UUserWidget* CreateLocalWidget(TSubclassOf<UUserWidget> WidgetClass);
};
