// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateBossCombatInfo.generated.h"

/**
 * 주기적으로 전투 정보를 갱신하고, BossCombatComponent 상태를 Blackboard로 미러링한다.
 *
 * 이 서비스는 패턴을 "선택"하지 않는다. 정보 갱신과 미러링만 담당한다.
 *   - TargetActor / DistanceToTarget
 *   - PatternLocked / IsInRecovery / InExecuteRange / CurrentPattern
 *
 * 각 Selector는 BT 에디터의 Details 패널에서 실제 Blackboard 키를 드롭다운으로 선택한다.
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
};
