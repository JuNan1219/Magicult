// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateBossCombatInfo.generated.h"

/**
 * 주기적으로 전투 정보를 갱신하고 BossCombatComponent 상태를 Blackboard로 미러링한다.
 * 패턴을 "선택"하지는 않는다 — 정보 갱신/미러링 전용이며, 각 Selector는 BT 에디터에서 개별 지정한다.
 */
UCLASS()
class MAGICULT_API UBTService_UpdateBossCombatInfo : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateBossCombatInfo();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector TargetActorSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector DistanceToTargetSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector PatternLockedSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector IsInRecoverySelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector InExecuteRangeSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector CurrentPatternSelector;

	// 새 타겟을 고를 때(패턴이 잠겨있지 않을 때) 최단거리 대상을 고를 확률. 1이면 항상 최단거리,
	// 낮을수록 플레이어 2명 사이에서 타겟이 갈릴 확률이 높아진다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ClosestTargetBias = 0.6f;
};
