// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultGameModeBase.h"
#include "MagicultGameState.h"
#include "MagicultGameInstance.h"
#include "Magicult/Character/MagicultEnemy.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

AMagicultGameModeBase::AMagicultGameModeBase()
{
	// MatchState 복제를 위해 AGameState 상속 GameState 사용
	GameStateClass = AMagicultGameState::StaticClass();
}

float AMagicultGameModeBase::GetEnemyHealthMultiplier()
{
	const int32 PlayerCount = FMath::Max(GetNumPlayers(), 1);
	return 1.f + (PlayerCount - 1) * HealthPerExtraPlayer;
}

int32 AMagicultGameModeBase::GetEffectiveExpectedCount()
{
	// 로비를 경유했으면 GameInstance에 기록된 기대 인원수를 분모로 사용.
	// 미설정(0 이하)이면 로비 미경유(직접 PIE/Standalone)로 보고 현재 접속 인원으로 대체한다.
	if (const UMagicultGameInstance* GI = Cast<UMagicultGameInstance>(GetGameInstance()))
	{
		if (GI->ExpectedPlayerCount > 0)
		{
			return GI->ExpectedPlayerCount;
		}
	}
	return GetNumPlayers();
}

bool AMagicultGameModeBase::ReadyToStartMatch_Implementation()
{
	if (GetMatchState() != MatchState::WaitingToStart)
	{
		return false;
	}

	// 타임아웃 강제 시작: 최소 인원만 충족하면 시작
	if (bForceStartByTimeout)
	{
		return GetNumPlayers() >= MinPlayersToStart;
	}

	// 전원 준비돼도 최소 대기 시간이 지나야 시작(대기 화면 확인/완충용). 0이면 즉시.
	if (MinWaitingSeconds > 0.f && GetWorld()->GetTimeSeconds() - WaitingStartTime < MinWaitingSeconds)
	{
		return false;
	}

	// 정상 게이트: 심리스 트래블 중인 플레이어가 모두 도착(NumTravellingPlayers==0)하고
	// 접속 인원이 기대 인원수 이상. 심리스 트래블에서는 클라가 맵 로딩+재접속을 마쳐야
	// travelling에서 빠지므로, 별도 클라 ready RPC 없이 이 둘로 "전원 도착"을 판정한다.
	// (NumPlayers >= ExpectedCount가 핵심 가드라, 트래블 카운트의 일시적 레이스도 흡수한다.)
	return NumTravellingPlayers == 0
		&& GetNumPlayers() > 0
		&& GetNumPlayers() >= GetEffectiveExpectedCount();
}

void AMagicultGameModeBase::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	WaitingStartTime = GetWorld()->GetTimeSeconds();

	// 한 명이 로딩에서 멈춰도 영구 대기에 빠지지 않도록 타임아웃 설정
	if (StartTimeoutSeconds > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			StartTimeoutTimerHandle,
			this,
			&AMagicultGameModeBase::OnStartTimeout,
			StartTimeoutSeconds,
			false);
	}
}

void AMagicultGameModeBase::OnStartTimeout()
{
	if (GetMatchState() != MatchState::WaitingToStart)
	{
		return;
	}

	if (GetNumPlayers() >= MinPlayersToStart)
	{
		// 현재 인원으로 강제 시작(ready 미통지 플레이어 폰도 RestartPlayer로 스폰됨).
		// 플래그를 세워 ReadyToStartMatch 폴링과 일관되게 만들고, 즉시성을 위해 StartMatch도 직접 호출.
		bForceStartByTimeout = true;
		StartMatch();
	}
	else
	{
		// 최소 인원 미달(예: 대기 중 전원 이탈) — 매치 미시작, 전원 메인 메뉴로 귀환
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				PC->ClientTravel(TEXT("/Game/Level/MainMenuMap"), TRAVEL_Absolute);
			}
		}
	}
}

void AMagicultGameModeBase::HandleMatchHasStarted()
{
	GetWorldTimerManager().ClearTimer(StartTimeoutTimerHandle);

	// AGameMode가 폰 미보유 플레이어를 RestartPlayer로 스폰+possess
	Super::HandleMatchHasStarted();

	// 모든 활성화를 InProgress 한 시점에 모은다: 배치 몬스터 일괄 초기화(체력 배율 + BT 시작)
	const float HealthMultiplier = GetEnemyHealthMultiplier();
	for (TActorIterator<AMagicultEnemy> It(GetWorld()); It; ++It)
	{
		if (AMagicultEnemy* Enemy = *It)
		{
			Enemy->InitializeForMatch(HealthMultiplier);
		}
	}
}

void AMagicultGameModeBase::Logout(AController* Exiting)
{
	// 대기 중 이탈이면 기대 인원수를 줄여 영구 대기를 방지
	if (GetMatchState() == MatchState::WaitingToStart)
	{
		if (UMagicultGameInstance* GI = Cast<UMagicultGameInstance>(GetGameInstance()))
		{
			if (GI->ExpectedPlayerCount > 0)
			{
				GI->ExpectedPlayerCount--;
			}
		}
	}

	Super::Logout(Exiting);
}
