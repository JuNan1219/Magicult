// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossPatternTypes.h"
#include "BossCombatComponent.generated.h"

/**
 * 보스 전투 상태의 단일 소스(Single Source of Truth). BehaviorTree 노드가 이 컴포넌트를
 * 읽고/변형하며, 분기에 필요한 최소값만 Blackboard로 미러링한다. 이 클래스는 Blackboard/BT를
 * 전혀 알지 못한다(순수 로직 → 테스트 용이). BT가 서버에서만 도는 것과 동일하게, 모든 상태
 * 변형은 서버 권한에서 호출된다는 전제.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MAGICULT_API UBossCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBossCombatComponent();

	// --- 조회 ---

	const FBossAttackPatternData* FindPatternData(EBossPattern Pattern) const;

	bool IsInExecuteRange(EBossPattern Pattern, float Distance) const;

	// 실행 최소 거리보다 안쪽으로 붙어버린 경우(Dash/JumpSmash가 너무 가까운 상황).
	bool IsTooClose(EBossPattern Pattern, float Distance) const;

	bool IsOnCooldown(EBossPattern Pattern) const;

	bool IsPatternLocked() const { return bPatternLocked; }
	EBossPattern GetCurrentPattern() const { return CurrentPattern; }

	// 패턴을 골랐지만 PatternTimeout 안에 실행 못 함.
	bool IsPatternTimedOut() const;

	// 공격 후딜 중인가. RecoveryEndTime과 현재 시각 비교(별도 타이머 없음).
	bool IsInRecovery() const;

	// --- 변형 (서버) ---

	// 점수를 확률로 변환해 패턴 선택. 성공 시 CurrentPattern/PatternLocked/PatternStartTime 설정.
	// 고를 게 없으면 None 반환(잠그지 않음).
	EBossPattern SelectPattern(float Distance);

	// 공격 실행 완료. 쿨다운/Recovery 시작, LastPattern/연속카운트 갱신, 잠금 해제.
	void OnPatternExecuted(EBossPattern Pattern);

	// 타임아웃/너무 가까움 등으로 패턴 포기. 다음 재선택에서 방금 포기한 패턴을 제외한다.
	void AbandonPattern();

protected:
	// 편집 가능한 패턴 테이블. 초기에는 Swing 하나만 채워도 된다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Patterns")
	TArray<FBossAttackPatternData> Patterns;

	// --- 확률 선택 파라미터 ---
	// 두 노브의 역할은 완전히 분리되어 있다.
	//   SelectionMargin      = "어디까지 후보로 볼 것인가" (컷 위치)
	//   SelectionTemperature = "점수 차를 얼마나 크게 볼 것인가" (분포 모양)

	// 최고 점수보다 이만큼 이상 낮은 패턴은 추첨에서 제외한다.
	// ScorePattern의 하드 배제 감점(-1000)/반복 감점(-50)을 걸러내는 역할도 겸한다.
	// DistanceWeight보다 크게 잡으면 거리 판단이 사실상 무력해지므로 그보다 작게 유지할 것.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Patterns", meta = (ClampMin = "0.0"))
	float SelectionMargin = 30.f;

	// 점수 차를 지수 가중치로 변환할 때의 온도. 작을수록 고점수에 쏠리고, 클수록 균등해진다.
	// 0이면 확률 선택을 끄고 결정론적 최고점 선택(argmax)으로 동작한다. 디버깅/롤백용.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Patterns", meta = (ClampMin = "0.0"))
	float SelectionTemperature = 10.f;

private:
	float ScorePattern(const FBossAttackPatternData& Data, float Distance) const;
	float GetNow() const;

	// --- 런타임 상태 (서버) ---
	EBossPattern CurrentPattern = EBossPattern::None;
	bool bPatternLocked = false;
	float PatternStartTime = 0.f;

	float RecoveryEndTime = 0.f;

	EBossPattern LastPattern = EBossPattern::None;
	int32 ConsecutiveSwing = 0;

	// 방금 포기(timeout/too close)한 패턴. 바로 다음 재선택에서만 제외한다.
	EBossPattern LastAbandonedPattern = EBossPattern::None;

	TMap<EBossPattern, float> CooldownEndTimes;
};
