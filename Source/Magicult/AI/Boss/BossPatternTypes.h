// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BossPatternTypes.generated.h"

class UGameplayAbility;

/** 보스가 사용할 수 있는 공격 패턴 종류. */
UENUM(BlueprintType)
enum class EBossPattern : uint8
{
	None,
	Swing,
	Dash,
	JumpSmash
};

/**
 * 패턴 1종의 정적 데이터.
 *
 * 초기에는 DataAsset 없이 UBossCombatComponent가 직접 배열로 보관한다.
 * 나중에 CharacterClassInfo/ElementInteractionRules처럼 DataAsset으로 승격할 수 있다.
 *
 * 거리 조건(ExecuteMin/MaxRange)은 "지금 당장 실행 가능한가"에만 쓴다.
 * "이번에 이 패턴을 고를 수 있는가"는 점수 계산(BaseScore/DistanceWeight 등)으로 판단한다.
 */
USTRUCT(BlueprintType)
struct FBossAttackPatternData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBossPattern Pattern = EBossPattern::None;

	// --- 실행 거리 (하드 조건) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range")
	float ExecuteMinRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range")
	float ExecuteMaxRange = 300.f;

	// --- 타이밍 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	float Cooldown = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	float Recovery = 0.5f;

	// 패턴을 골랐지만 실행 거리까지 못 붙는 상황을 끊기 위한 타임아웃.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	float PatternTimeout = 2.f;

	// --- 선택 점수 (소프트 조건) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score")
	float BaseScore = 50.f;

	// 실행 구간 중심에 가까울수록 가점, 멀수록 감점되는 가중치(이동 비용 반영).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score")
	float DistanceWeight = 40.f;

	// 직전에 실행한 패턴과 같아도 연속 허용할지. Swing만 true 권장.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score")
	bool bAllowConsecutive = false;

	// 연속 허용 패턴이라도 이 횟수 이상 반복되면 감점.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score")
	int32 MaxConsecutive = 3;

	// --- 실행 ---
	// BT가 발행할 GameplayEvent 태그. 이 태그로 트리거되는 GA가 실제 공격을 담당한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execute")
	FGameplayTag AttackEventTag;

	// 이 패턴이 실제로 활성화하는 GA 클래스. ExecuteSelectedPattern이 "이 클래스의 GA가
	// 끝났을 때만 종료로 인정"하는 필터로 쓴다(HitReact 등 다른 GA 종료와 오탐 방지).
	// 패턴마다 GA가 다르므로 BT 노드가 아니라 여기(패턴 데이터)에 둔다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Execute")
	TSubclassOf<UGameplayAbility> AbilityClass;
};
