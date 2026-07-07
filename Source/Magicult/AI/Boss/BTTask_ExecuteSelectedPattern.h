// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "BossPatternTypes.h"
#include "BTTask_ExecuteSelectedPattern.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

/**
 * 선택된 패턴을 실제로 실행 요청한다. (BTTask_Attack과 동일한 GA 흐름을 패턴 데이터 기반으로 일반화)
 *
 *  1) StopMovement + FaceTarget
 *  2) CurrentPattern의 AttackEventTag로 GameplayEvent 발행 → GA 활성화
 *  3) 어빌리티 종료(OnAbilityEnded)를 감지해 종료 → BossCombatComponent::OnPatternExecuted
 *
 * 종료 인정 필터(어떤 GA 클래스가 끝나야 하는지)는 FBossAttackPatternData::AbilityClass에서
 * 실행 중인 패턴 기준으로 매번 가져온다(패턴마다 GA가 다르므로 노드에 고정하지 않는다).
 * 비워두면(None) 활성화 이후 첫 어빌리티 종료에서 끝난다.
 *
 * 주의: OnAbilityEnded는 BT 틱 스택 밖(GA 종료 시점)에서 비동기로 불리는 델리게이트라,
 * UBTFunctionLibrary 헬퍼 대신 캐싱해둔 OwnerComp에서 직접 Blackboard를 얻어 쓴다.
 */
UCLASS()
class MAGICULT_API UBTTask_ExecuteSelectedPattern : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ExecuteSelectedPattern();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector TargetActorSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector PatternLockedSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector CurrentPatternSelector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard")
	FBlackboardKeySelector IsInRecoverySelector;

private:
	void OnAbilityEnded(const FAbilityEndedData& EndedData);
	void CleanupDelegate();

	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	FDelegateHandle AbilityEndedHandle;
	EBossPattern ExecutingPattern = EBossPattern::None;

	// ExecuteTask 시점에 CurrentPattern의 AbilityClass를 캐싱해, 종료 필터링에 사용한다.
	TSubclassOf<UGameplayAbility> ExecutingAbilityClass;
};
