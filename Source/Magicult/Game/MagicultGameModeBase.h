// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MagicultGameState.h"
#include "MagicultGameModeBase.generated.h"

class UMagicInfo;
class UTagToData;
class UElementInteractionRules;
class UProjectileData;
class UCharacterClassInfo;
class UGameplayEffect;
class AController;
class AMagicultEnemy;

/**
 *
 */
UCLASS()
class MAGICULT_API AMagicultGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:
	AMagicultGameModeBase();

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UMagicInfo> MagicInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UTagToData> TagToData;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UElementInteractionRules> ElementInteractionRules;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UProjectileData> ProjectileData;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage")
	TSubclassOf<UGameplayEffect> DefaultDamageGameplayEffect;

	// 몬스터 체력 인원수 배율: 1명당 기본, 추가 1명마다 (1 + 추가인원 * HealthPerExtraPlayer)배.
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Scaling")
	float HealthPerExtraPlayer = 1.0f;

	// 매치를 시작하기 위한 최소 접속 인원. 타임아웃 폴백 판정에도 사용.
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 MinPlayersToStart = 1;

	// WaitingToStart에서 전원 ready를 기다리는 최대 시간(초). 도달 시 현재 인원으로 강제 진행.
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float StartTimeoutSeconds = 60.f;

	// 전원 준비돼도 최소 이 시간(초)만큼은 WaitingToStart를 유지한다(대기 화면 확인/완충용).
	// 0이면 준비되는 즉시 시작.
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float MinWaitingSeconds = 0.f;

	// 보스 처치 시 승리로 매치를 끝낼지.
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	bool bEndMatchOnBossDefeated = true;

	// 전원 사망 시 패배로 매치를 끝낼지. 테스트 부활(bTestRespawnEnabled)이 걸린 플레이어는
	// "곧 돌아옴"으로 보고 전멸 판정에서 제외하므로, 부활이 켜진 동안은 사실상 패배하지 않는다.
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	bool bEndMatchOnPartyWipe = true;

	// 현재 접속 인원 기준 몬스터 체력 배율을 산정한다(서버).
	float GetEnemyHealthMultiplier();

	// 보스가 죽으면 보스 자신이 호출한다(서버). 살아있는 보스가 더 없으면 승리로 끝낸다.
	void NotifyBossDefeated(AMagicultEnemy* DefeatedBoss);

	// 플레이어가 죽으면 캐릭터가 호출한다(서버). 전멸이면 패배로 끝낸다.
	void NotifyPlayerDied(APawn* DeadPawn);

	// 결과를 확정하고 매치를 끝낸다. InProgress가 아니면 무시한다(중복 종료 가드).
	void EndMatchWithOutcome(EMatchOutcome Outcome);

protected:
	virtual bool ReadyToStartMatch_Implementation() override;
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	virtual void Logout(AController* Exiting) override;

private:
	// 로비에서 기록한 기대 인원수(분모). 0 이하이면 로비 미경유로 보고 GetNumPlayers()로 대체.
	int32 GetEffectiveExpectedCount();

	FTimerHandle StartTimeoutTimerHandle;
	bool bForceStartByTimeout = false;

	// WaitingToStart에 진입한 시각(MinWaitingSeconds 판정용).
	float WaitingStartTime = 0.f;

	void OnStartTimeout();

	// EndMatchWithOutcome이 기록해두고, HandleMatchHasEnded가 GameState로 흘려보낸다.
	// (AGameMode::EndMatch는 사유를 전달하지 못하므로 한 프레임 보관이 필요하다.)
	EMatchOutcome PendingOutcome = EMatchOutcome::None;

	// 살아있는(또는 부활 예정인) 플레이어가 하나도 없으면 true.
	bool IsPartyWiped() const;
};
