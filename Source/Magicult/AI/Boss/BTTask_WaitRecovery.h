// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitRecovery.generated.h"

/**
 * 공격 후딜(Recovery)이 끝날 때까지 대기한다.
 * 판정은 UBossCombatComponent::IsInRecovery(RecoveryEndTime 비교)에 위임한다.
 */
UCLASS()
class MAGICULT_API UBTTask_WaitRecovery : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WaitRecovery();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector IsInRecoverySelector;

private:
	// Recovery가 끝났으면 Blackboard를 내리고 Succeeded 여부를 반환.
	bool IsRecoveryFinished(UBehaviorTreeComponent& OwnerComp);
};
