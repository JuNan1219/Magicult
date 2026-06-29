// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultGameState.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

void AMagicultGameState::BeginPlay()
{
	Super::BeginPlay();

	// 기본적으로 매치 시작 전이면 대기 위젯을 띄운다. OnRep_MatchState 타이밍을 놓쳐도
	// "시작 전엔 보이고, InProgress에서 HandleMatchHasStarted가 내린다"가 보장된다.
	if (!IsMatchInProgress())
	{
		ShowWaitingWidget();
	}
}

void AMagicultGameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	ShowWaitingWidget();
}

void AMagicultGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	HideWaitingWidget();
}

void AMagicultGameState::ShowWaitingWidget()
{
	// 데디 서버는 로컬 플레이어가 없으므로 위젯을 만들지 않는다(리슨 호스트/클라만 표시).
	if (GetNetMode() == NM_DedicatedServer) return;
	if (WaitingWidget || !WaitingWidgetClass) return;
	if (IsMatchInProgress()) return;

	UGameInstance* GI = GetGameInstance();
	APlayerController* PC = GI ? GI->GetFirstLocalPlayerController() : nullptr;
	if (!PC)
	{
		// 로컬 PC가 아직 준비 안 됨(레이스). 잠시 후 재시도.
		GetWorldTimerManager().SetTimer(RetryTimerHandle, this, &AMagicultGameState::ShowWaitingWidget, 0.2f, false);
		return;
	}

	WaitingWidget = CreateWidget<UUserWidget>(PC, WaitingWidgetClass);
	if (WaitingWidget)
	{
		// 다른 HUD 위에 덮도록 높은 Z-Order.
		WaitingWidget->AddToViewport(1000);
	}
}

void AMagicultGameState::HideWaitingWidget()
{
	GetWorldTimerManager().ClearTimer(RetryTimerHandle);

	if (WaitingWidget)
	{
		WaitingWidget->RemoveFromParent();
		WaitingWidget = nullptr;
	}
}
