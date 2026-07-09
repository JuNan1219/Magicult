// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_UpdateBossCombatInfo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "BossCombatComponent.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/Interface/EnemyInterface.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"

UBTService_UpdateBossCombatInfo::UBTService_UpdateBossCombatInfo()
{
	NodeName = TEXT("Update Boss Combat Info");
	// 문서 권장 갱신 주기 0.1~0.2초.
	Interval = 0.15f;
	RandomDeviation = 0.02f;
}

void UBTService_UpdateBossCombatInfo::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!BB || !AICon)
	{
		return;
	}

	APawn* OwningPawn = AICon->GetPawn();
	if (!IsValid(OwningPawn))
	{
		return;
	}

	UBossCombatComponent* Combat = OwningPawn->FindComponentByClass<UBossCombatComponent>();

	// --- 타겟 후보 탐색 (파벌 태그 기반 적 전체, 최대 2인) ---
	TArray<AActor*> CombatActors;
	UGameplayStatics::GetAllActorsWithInterface(OwningPawn, UCombatInterface::StaticClass(), CombatActors);

	TArray<AActor*> Candidates;
	float ClosestDistance = TNumericLimits<float>::Max();
	AActor* ClosestActor = nullptr;
	for (AActor* Actor : CombatActors)
	{
		if (!IsValid(Actor) || Actor == OwningPawn) continue;
		if (ICombatInterface::Execute_IsDead(Actor)) continue;
		if (!UMagicultAbilitySystemLibrary::IsNotFriend(OwningPawn, Actor)) continue;

		Candidates.Add(Actor);
		const float Distance = OwningPawn->GetDistanceTo(Actor);
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestActor = Actor;
		}
	}

	// 패턴 실행 중(잠긴 상태)에는 공격 도중 타겟이 바뀌지 않도록 기존 타겟을 유지하고,
	// 잠기지 않았을 때(다음 패턴을 고르는 시점)만 재선정한다 — 그래야 매 틱(0.15초)마다
	// 흔들리지 않고, 공격이 끝날 때마다 자연스럽게 다른 플레이어로 갈릴 수 있다.
	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetActorSelector.SelectedKeyName));
	const bool bKeepCurrentTarget = Combat && Combat->IsPatternLocked() && Candidates.Contains(CurrentTarget);

	AActor* SelectedTarget = nullptr;
	if (bKeepCurrentTarget)
	{
		SelectedTarget = CurrentTarget;
	}
	else if (Candidates.Num() > 0)
	{
		// ClosestTargetBias 확률로 최단거리 대상을, 그 외에는 후보 중 무작위로 선택한다.
		SelectedTarget = (FMath::FRand() < ClosestTargetBias)
			? ClosestActor
			: Candidates[FMath::RandHelper(Candidates.Num())];
	}

	// NodeOwner(this) 기반 UBTFunctionLibrary 헬퍼는 이 노드처럼 인스턴스가 공유되는
	// (bCreateNodeInstance=false) Service/Task에서 소유 BT 컴포넌트를 못 찾는다.
	// 항상 파라미터로 들어오는 OwnerComp에서 직접 Blackboard를 얻어 쓴다.
	BB->SetValueAsObject(TargetActorSelector.SelectedKeyName, SelectedTarget);

	// GA가 Blackboard가 아니라 CombatTarget(IEnemyInterface)을 읽어서 공격 대상을 찾으므로,
	// 여기서도 같이 채워준다. 안 그러면 GA 안에서 GetCombatTarget()이 nullptr로 나온다.
	if (OwningPawn->Implements<UEnemyInterface>())
	{
		IEnemyInterface::Execute_SetCombatTarget(OwningPawn, SelectedTarget);
	}

	if (!SelectedTarget)
	{
		BB->SetValueAsFloat(DistanceToTargetSelector.SelectedKeyName, TNumericLimits<float>::Max());
		return;
	}

	const float SelectedDistance = OwningPawn->GetDistanceTo(SelectedTarget);
	BB->SetValueAsFloat(DistanceToTargetSelector.SelectedKeyName, SelectedDistance);

	// --- BossCombatComponent 상태 미러링 ---
	if (Combat)
	{
		const EBossPattern Current = Combat->GetCurrentPattern();
		BB->SetValueAsBool(PatternLockedSelector.SelectedKeyName, Combat->IsPatternLocked());
		BB->SetValueAsBool(IsInRecoverySelector.SelectedKeyName, Combat->IsInRecovery());
		BB->SetValueAsBool(InExecuteRangeSelector.SelectedKeyName, Combat->IsInExecuteRange(Current, SelectedDistance));
		BB->SetValueAsEnum(CurrentPatternSelector.SelectedKeyName, static_cast<uint8>(Current));
	}
}
