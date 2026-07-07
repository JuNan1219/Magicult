// Fill out your copyright notice in the Description page of Project Settings.

#include "BossCombatComponent.h"
#include "Engine/World.h"

UBossCombatComponent::UBossCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UBossCombatComponent::GetNow() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.f;
}

const FBossAttackPatternData* UBossCombatComponent::FindPatternData(EBossPattern Pattern) const
{
	if (Pattern == EBossPattern::None)
	{
		return nullptr;
	}
	return Patterns.FindByPredicate([Pattern](const FBossAttackPatternData& Data)
		{
			return Data.Pattern == Pattern;
		});
}

bool UBossCombatComponent::IsInExecuteRange(EBossPattern Pattern, float Distance) const
{
	const FBossAttackPatternData* Data = FindPatternData(Pattern);
	if (!Data)
	{
		return false;
	}
	return Distance >= Data->ExecuteMinRange && Distance <= Data->ExecuteMaxRange;
}

bool UBossCombatComponent::IsTooClose(EBossPattern Pattern, float Distance) const
{
	const FBossAttackPatternData* Data = FindPatternData(Pattern);
	return Data && Distance < Data->ExecuteMinRange;
}

bool UBossCombatComponent::IsOnCooldown(EBossPattern Pattern) const
{
	const float* EndTime = CooldownEndTimes.Find(Pattern);
	return EndTime && GetNow() < *EndTime;
}

bool UBossCombatComponent::IsPatternTimedOut() const
{
	if (!bPatternLocked)
	{
		return false;
	}
	const FBossAttackPatternData* Data = FindPatternData(CurrentPattern);
	if (!Data)
	{
		return false;
	}
	return (GetNow() - PatternStartTime) > Data->PatternTimeout;
}

bool UBossCombatComponent::IsInRecovery() const
{
	return GetNow() < RecoveryEndTime;
}

float UBossCombatComponent::ScorePattern(const FBossAttackPatternData& Data, float Distance) const
{
	float Score = Data.BaseScore;

	// 거리 보정: 실행 구간 중심에 가까울수록 가점, 멀수록 감점(이동 비용 반영).
	// 하드 컷이 아니라 소프트 가중치이므로, 멀어도 점수는 낮아질 뿐 후보에서 탈락하지 않는다.
	const float Center = (Data.ExecuteMinRange + Data.ExecuteMaxRange) * 0.5f;
	const float HalfSpan = FMath::Max(1.f, (Data.ExecuteMaxRange - Data.ExecuteMinRange) * 0.5f);
	const float Normalized = FMath::Abs(Distance - Center) / HalfSpan; // 0 = 중심
	Score += Data.DistanceWeight * FMath::Clamp(1.f - Normalized, -1.f, 1.f);

	// 반복 방지: 직전과 같은 패턴이고 연속 불가면 사실상 제외.
	if (Data.Pattern == LastPattern && !Data.bAllowConsecutive)
	{
		Score -= 1000.f;
	}

	// 연속 허용 패턴(Swing)이라도 너무 반복되면 감점하여 다른 패턴에 기회를 준다.
	if (Data.bAllowConsecutive && Data.Pattern == LastPattern && ConsecutiveSwing >= Data.MaxConsecutive)
	{
		Score -= 50.f;
	}

	return Score;
}

EBossPattern UBossCombatComponent::SelectPattern(float Distance)
{
	EBossPattern Best = EBossPattern::None;
	float BestScore = -FLT_MAX;

	for (const FBossAttackPatternData& Data : Patterns)
	{
		if (Data.Pattern == EBossPattern::None)
		{
			continue;
		}
		// 하드 제외: 쿨다운 중 / 방금 포기한 패턴.
		if (IsOnCooldown(Data.Pattern))
		{
			continue;
		}
		if (Data.Pattern == LastAbandonedPattern)
		{
			continue;
		}

		// 점수가 음수여도 배제하지 않는다(문서 원칙: 거리/반복 감점은 소프트 값).
		// 다른 대안이 없으면 감점된 패턴이라도 선택되어야, 유일한 생존 후보가
		// "직전과 같아서 연속 불가" 하나뿐일 때 보스가 그냥 멈춰버리는 것을 막는다.
		const float Score = ScorePattern(Data, Distance);
		if (Score > BestScore)
		{
			BestScore = Score;
			Best = Data.Pattern;
		}
	}

	if (Best != EBossPattern::None)
	{
		CurrentPattern = Best;
		bPatternLocked = true;
		PatternStartTime = GetNow();
	}
	return Best;
}

void UBossCombatComponent::OnPatternExecuted(EBossPattern Pattern)
{
	const float Now = GetNow();
	if (const FBossAttackPatternData* Data = FindPatternData(Pattern))
	{
		CooldownEndTimes.Add(Pattern, Now + Data->Cooldown);
		RecoveryEndTime = Now + Data->Recovery;
	}

	// 연속 카운트: Swing 계열(연속 허용)만 누적, 그 외는 리셋.
	if (Pattern == LastPattern)
	{
		++ConsecutiveSwing;
	}
	else
	{
		ConsecutiveSwing = 1;
	}

	LastPattern = Pattern;
	LastAbandonedPattern = EBossPattern::None;
	CurrentPattern = EBossPattern::None;
	bPatternLocked = false;
}

void UBossCombatComponent::AbandonPattern()
{
	LastAbandonedPattern = CurrentPattern;
	CurrentPattern = EBossPattern::None;
	bPatternLocked = false;
}
