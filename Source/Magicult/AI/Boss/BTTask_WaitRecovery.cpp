// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_WaitRecovery.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "BossCombatComponent.h"

UBTTask_WaitRecovery::UBTTask_WaitRecovery()
{
	NodeName = TEXT("Wait Recovery");
	bNotifyTick = true;
}

bool UBTTask_WaitRecovery::IsRecoveryFinished(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	UBossCombatComponent* Combat = Pawn ? Pawn->FindComponentByClass<UBossCombatComponent>() : nullptr;
	if (!Combat)
	{
		return true; // 컴포넌트가 없으면 대기할 이유가 없음.
	}

	if (Combat->IsInRecovery())
	{
		return false;
	}

	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsBool(IsInRecoverySelector.SelectedKeyName, false);
	}
	return true;
}

EBTNodeResult::Type UBTTask_WaitRecovery::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return IsRecoveryFinished(OwnerComp) ? EBTNodeResult::Succeeded : EBTNodeResult::InProgress;
}

void UBTTask_WaitRecovery::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (IsRecoveryFinished(OwnerComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
