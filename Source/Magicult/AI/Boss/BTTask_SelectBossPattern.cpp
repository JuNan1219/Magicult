// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_SelectBossPattern.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "BossCombatComponent.h"

UBTTask_SelectBossPattern::UBTTask_SelectBossPattern()
{
	NodeName = TEXT("Select Boss Pattern");
}

EBTNodeResult::Type UBTTask_SelectBossPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	if (!BB || !Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UBossCombatComponent* Combat = Pawn->FindComponentByClass<UBossCombatComponent>();
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorSelector.SelectedKeyName));
	if (!Combat || !Target)
	{
		return EBTNodeResult::Failed;
	}

	const float Distance = Pawn->GetDistanceTo(Target);
	const EBossPattern Picked = Combat->SelectPattern(Distance);
	if (Picked == EBossPattern::None)
	{
		// 이번 틱에 고를 수 있는 패턴이 없음. 다음 틱에 다시 시도.
		return EBTNodeResult::Failed;
	}

	// 서비스 미러링을 기다리지 않고 즉시 반영(같은 프레임 재평가 시 stale 방지).
	BB->SetValueAsEnum(CurrentPatternSelector.SelectedKeyName, static_cast<uint8>(Picked));
	BB->SetValueAsBool(PatternLockedSelector.SelectedKeyName, true);
	return EBTNodeResult::Succeeded;
}
