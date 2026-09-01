// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToPatternRange.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "BossCombatComponent.h"

UBTTask_MoveToPatternRange::UBTTask_MoveToPatternRange()
{
	NodeName = TEXT("Move To Pattern Range");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToPatternRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EvaluateAndMaybeFinish(OwnerComp, /*bAllowIssueMove=*/true);
}

void UBTTask_MoveToPatternRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	const EBTNodeResult::Type Result = EvaluateAndMaybeFinish(OwnerComp, /*bAllowIssueMove=*/false);
	if (Result != EBTNodeResult::InProgress)
	{
		FinishLatentTask(OwnerComp, Result);
	}
}

void UBTTask_MoveToPatternRange::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// Succeeded/Failed/Aborted 어떤 경로로 끝나든 이동은 항상 멈춘다.
	// (HitReact 데코레이터 등 외부 Abort 시에도 이동이 계속되지 않도록 보장)
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		AICon->StopMovement();
	}

	// 외부에서 중단된 경우(Succeeded/Failed는 이미 EvaluateAndMaybeFinish 안에서 필요한 정리를 마쳤음)만
	// 여기서 패턴을 포기 처리한다. 안 그러면 중단된 채로 PatternLocked가 계속 true로 남아
	// Combat 재개 후에도 새 패턴을 고르지 못하게 된다.
	if (TaskResult == EBTNodeResult::Aborted)
	{
		AbandonAndUnlock(OwnerComp);
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

EBTNodeResult::Type UBTTask_MoveToPatternRange::EvaluateAndMaybeFinish(UBehaviorTreeComponent& OwnerComp, bool bAllowIssueMove)
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

	const EBossPattern Pattern = Combat->GetCurrentPattern();
	if (Pattern == EBossPattern::None)
	{
		// 패턴이 없으면 FindPatternData가 항상 nullptr이라 이하 판정이 전부 false로 빠져서
		// AcceptRadius 기본값(5cm)으로 MoveToActor만 계속 쏘며 영원히 InProgress로 남는다.
		// 정상 흐름에선 SelectPattern이 먼저 잠가야 이 노드가 도니 도달하면 안 되지만,
		// 방어적으로 즉시 Failed 처리한다.
		return EBTNodeResult::Failed;
	}

	// 블랙보드 미러값(서비스, 최대 0.15s 지연 + 초기 0 위험)에 의존하지 않고 현재 거리를 직접 계산한다.
	const float Distance = Pawn->GetDistanceTo(Target);

	// 타임아웃: 패턴을 골랐지만 사거리에 못 붙음.
	if (Combat->IsPatternTimedOut())
	{
		AbandonAndUnlock(OwnerComp);
		return EBTNodeResult::Failed;
	}

	// 실행 거리 안에 들어옴 → 성공.
	if (Combat->IsInExecuteRange(Pattern, Distance))
	{
		// 서비스의 다음 갱신(최대 0.15s)을 기다리지 않고 즉시 반영한다.
		// 이게 없으면 Move 성공 이후 재평가 시 InExecuteRange가 아직 false라서
		// ExecutePattern 데코를 못 통과하고 MoveToPatternRange가 재실행되는 상황이 생긴다.
		BB->SetValueAsBool(InExecuteRangeSelector.SelectedKeyName, true);
		return EBTNodeResult::Succeeded;
	}

	// 너무 가까움(실행 최소 거리 안쪽) → 초기 구현에서는 재선택으로 넘긴다.
	if (Combat->IsTooClose(Pattern, Distance))
	{
		AbandonAndUnlock(OwnerComp);
		return EBTNodeResult::Failed;
	}

	// 아직 멀다 → 이동. 이동 중이 아니면 (재)요청.
	if (bAllowIssueMove || AICon->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		float AcceptRadius = 5.f;
		if (const FBossAttackPatternData* Data = Combat->FindPatternData(Pattern))
		{
			AcceptRadius = FMath::Max(5.f, Data->ExecuteMaxRange - AcceptanceRadiusPadding);
		}

		// bStopOnOverlap=false: 타겟 콜리전에 닿는 순간이 아니라, 우리가 지정한 AcceptRadius
		// (실행 사거리 기준)로만 정지 판정을 하도록 한다. 콜리전 크기에 따라 일찍 멈추는 걸 방지.
		const EPathFollowingRequestResult::Type MoveResult =
			AICon->MoveToActor(Target, AcceptRadius, /*bStopOnOverlap=*/false, /*bUsePathfinding=*/true);

		if (MoveResult == EPathFollowingRequestResult::Failed)
		{
			// 경로 생성 실패(NavMesh 없음/도달 불가 등)를 조용히 넘기지 않는다. 제자리 무한 재요청 방지.
			UE_LOG(LogTemp, Warning,
				TEXT("[Boss] MoveToPatternRange: MoveToActor 실패 — NavMesh/도달성 확인 필요 (%s → %s)"),
				*Pawn->GetName(), *Target->GetName());
			AbandonAndUnlock(OwnerComp);
			return EBTNodeResult::Failed;
		}
		else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			// 이미 AcceptRadius 안이라 실제 이동 요청이 발동되지 않은 경우.
			// 이걸 그냥 두면 GetMoveStatus()가 계속 Idle이라 매 틱 MoveToActor만 헛되이
			// 재호출하며 멈춰 있게 된다(우리 IsInExecuteRange 판정과 반경이 달라 생기는 간극).
			// 이미 목표에 도달한 것으로 보고 바로 성공 처리한다.
			BB->SetValueAsBool(InExecuteRangeSelector.SelectedKeyName, true);
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToPatternRange::AbandonAndUnlock(UBehaviorTreeComponent& OwnerComp)
{
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		if (APawn* Pawn = AICon->GetPawn())
		{
			if (UBossCombatComponent* Combat = Pawn->FindComponentByClass<UBossCombatComponent>())
			{
				Combat->AbandonPattern();
			}
		}
	}
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsBool(PatternLockedSelector.SelectedKeyName, false);
		BB->SetValueAsBool(InExecuteRangeSelector.SelectedKeyName, false);
		// 컴포넌트의 CurrentPattern은 AbandonPattern()에서 이미 None으로 리셋됐지만,
		// 블랙보드 미러는 서비스의 다음 갱신(최대 0.15s) 전까지 이전 패턴 값으로 stale하게 남는다.
		BB->SetValueAsEnum(CurrentPatternSelector.SelectedKeyName, static_cast<uint8>(EBossPattern::None));
	}
}
