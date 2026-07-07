// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SelectBossPattern.generated.h"

/**
 * PatternLocked == false일 때만 실행. 점수 기반으로 다음 패턴을 결정한다.
 *
 * 실제 선택 로직은 UBossCombatComponent::SelectPattern이 담당한다.
 * 이 Task는 결과를 Blackboard(CurrentPattern/PatternLocked)로 즉시 반영만 한다.
 */
UCLASS()
class MAGICULT_API UBTTask_SelectBossPattern : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SelectBossPattern();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 블랙보드 미러(서비스, 최대 0.15s 지연 + 초기 0 위험) 대신 이 키로 타겟을 얻어
	// 직접 거리를 계산한다(MoveToPatternRange와 동일한 이유).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector TargetActorSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector CurrentPatternSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector PatternLockedSelector;
};
