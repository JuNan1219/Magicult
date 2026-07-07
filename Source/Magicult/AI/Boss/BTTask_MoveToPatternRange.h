// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToPatternRange.generated.h"

/**
 * 선택된 패턴이 아직 실행 거리 밖이면 타겟 쪽으로 접근한다.
 *
 *  - 실행 거리 안에 들어오면 Succeeded
 *  - PatternTimeout 초과 / 실행 최소 거리보다 안쪽(too close)이면 패턴 포기 후 Failed
 *
 * "too close 시 뒤로 빠지는 이동"은 초기 구현에서 다루지 않고 재선택으로 넘긴다(문서 12장).
 */
UCLASS()
class MAGICULT_API UBTTask_MoveToPatternRange : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToPatternRange();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector TargetActorSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector PatternLockedSelector;

	// Abandon 시 블랙보드 미러값도 즉시 None으로 되돌리기 위한 키.
	// (컴포넌트의 CurrentPattern은 AbandonPattern()에서 이미 None으로 리셋되지만,
	// 블랙보드 미러는 서비스가 다음 갱신을 하기 전까지 stale하게 남는다)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector CurrentPatternSelector;

	// 서비스 갱신 주기(0.15s)를 기다리지 않도록, 사거리 도달 판정 성공 시 이 노드가 직접 true로 쓴다.
	// (서비스만 이 값을 쓰면 Move 성공과 ExecutePattern 데코 통과 사이에 레이스가 생길 수 있음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector InExecuteRangeSelector;

	// MoveTo 도착 판정 반경. 실행 최대 거리보다 살짝 안쪽에서 멈추도록 하는 안전 여유(cm).
	UPROPERTY(EditAnywhere, Category = "Move")
	float AcceptanceRadiusPadding = 30.f;

private:
	// 패턴 포기 처리 후 Blackboard PatternLocked를 내린다.
	void AbandonAndUnlock(UBehaviorTreeComponent& OwnerComp);

	// 현재 상황을 재평가해 종료해야 하면 결과를, 계속 이동해야 하면 InProgress를 반환.
	EBTNodeResult::Type EvaluateAndMaybeFinish(UBehaviorTreeComponent& OwnerComp, bool bAllowIssueMove);
};
