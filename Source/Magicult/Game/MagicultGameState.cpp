// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultGameState.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

void AMagicultGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMagicultGameState, MatchOutcome);
}

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

void AMagicultGameState::PlayWorldCue(FGameplayTag CueTag, const FGameplayCueParameters& Params)
{
	if (!HasAuthority() || !CueTag.IsValid())
	{
		return;
	}

	NetMulticast_PlayWorldCue(CueTag, Params);
}

void AMagicultGameState::NetMulticast_PlayWorldCue_Implementation(FGameplayTag CueTag, FGameplayCueParameters Params)
{
	// GameState는 위치를 가진 액터가 아니라, Location이 비면 엔진이 GameState 루트(에디터 전용
	// 스프라이트 = 월드 원점)로 폴백한다. 발신 측 가드가 뚫려도 여기서 막는다.
	if (Params.Location.IsZero())
	{
		return;
	}

	if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
	{
		CueManager->HandleGameplayCue(this, CueTag, EGameplayCueEvent::Executed, Params);
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

void AMagicultGameState::SetMatchOutcome(EMatchOutcome NewOutcome)
{
	if (!HasAuthority() || MatchOutcome == NewOutcome) return;

	MatchOutcome = NewOutcome;

	// 서버 자신에게는 OnRep이 호출되지 않으므로 직접 태운다(리슨 호스트 화면).
	OnRep_MatchOutcome();
}

void AMagicultGameState::OnRep_MatchOutcome()
{
	ShowResultWidget();
}

void AMagicultGameState::ShowWaitingWidget()
{
	if (WaitingWidget) return;
	if (IsMatchInProgress()) return;

	if (UUserWidget* Widget = CreateLocalWidget(WaitingWidgetClass))
	{
		WaitingWidget = Widget;
	}
	else if (WaitingWidgetClass && GetNetMode() != NM_DedicatedServer)
	{
		// 로컬 PC가 아직 준비 안 됨(레이스). 잠시 후 재시도.
		GetWorldTimerManager().SetTimer(RetryTimerHandle, this, &AMagicultGameState::ShowWaitingWidget, 0.2f, false);
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

void AMagicultGameState::ShowResultWidget()
{
	if (ResultWidget) return;

	const TSubclassOf<UUserWidget> WidgetClass =
		(MatchOutcome == EMatchOutcome::Victory) ? VictoryWidgetClass :
		(MatchOutcome == EMatchOutcome::Defeat) ? DefeatWidgetClass : nullptr;

	if (!WidgetClass) return;

	// 결과 화면이 대기 위젯 위에 겹치지 않도록 정리한다(시작 전에 끝나는 예외 경로 대비).
	HideWaitingWidget();

	if (UUserWidget* Widget = CreateLocalWidget(WidgetClass))
	{
		ResultWidget = Widget;
	}
	else if (GetNetMode() != NM_DedicatedServer)
	{
		GetWorldTimerManager().SetTimer(ResultRetryTimerHandle, this, &AMagicultGameState::ShowResultWidget, 0.2f, false);
	}
}

UUserWidget* AMagicultGameState::CreateLocalWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	// 데디 서버는 로컬 플레이어가 없으므로 위젯을 만들지 않는다(리슨 호스트/클라만 표시).
	if (GetNetMode() == NM_DedicatedServer) return nullptr;
	if (!WidgetClass) return nullptr;

	UGameInstance* GI = GetGameInstance();
	APlayerController* PC = GI ? GI->GetFirstLocalPlayerController() : nullptr;
	if (!PC) return nullptr;

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, WidgetClass);
	if (Widget)
	{
		// 다른 HUD 위에 덮도록 높은 Z-Order.
		Widget->AddToViewport(1000);
	}
	return Widget;
}
