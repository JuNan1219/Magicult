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

	// --- 타겟 탐색 (파벌 태그 기반, 가장 가까운 적) ---
	TArray<AActor*> CombatActors;
	UGameplayStatics::GetAllActorsWithInterface(OwningPawn, UCombatInterface::StaticClass(), CombatActors);

	float ClosestDistance = TNumericLimits<float>::Max();
	AActor* ClosestActor = nullptr;
	for (AActor* Actor : CombatActors)
	{
		if (!IsValid(Actor) || Actor == OwningPawn) continue;
		if (ICombatInterface::Execute_IsDead(Actor)) continue;
		if (!UMagicultAbilitySystemLibrary::IsNotFriend(OwningPawn, Actor)) continue;

		const float Distance = OwningPawn->GetDistanceTo(Actor);
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestActor = Actor;
		}
	}

	// NodeOwner(this) 기반 UBTFunctionLibrary 헬퍼는 이 노드처럼 인스턴스가 공유되는
	// (bCreateNodeInstance=false) Service/Task에서 소유 BT 컴포넌트를 못 찾는다.
	// 항상 파라미터로 들어오는 OwnerComp에서 직접 Blackboard를 얻어 쓴다.
	BB->SetValueAsObject(TargetActorSelector.SelectedKeyName, ClosestActor);

	// GA가 Blackboard가 아니라 CombatTarget(IEnemyInterface)을 읽어서 공격 대상을 찾으므로,
	// 여기서도 같이 채워준다. 안 그러면 GA 안에서 GetCombatTarget()이 nullptr로 나온다.
	if (OwningPawn->Implements<UEnemyInterface>())
	{
		IEnemyInterface::Execute_SetCombatTarget(OwningPawn, ClosestActor);
	}

	if (!ClosestActor)
	{
		BB->SetValueAsFloat(DistanceToTargetSelector.SelectedKeyName, TNumericLimits<float>::Max());
		return;
	}

	BB->SetValueAsFloat(DistanceToTargetSelector.SelectedKeyName, ClosestDistance);

	// --- BossCombatComponent 상태 미러링 ---
	if (const UBossCombatComponent* Combat = OwningPawn->FindComponentByClass<UBossCombatComponent>())
	{
		const EBossPattern Current = Combat->GetCurrentPattern();
		BB->SetValueAsBool(PatternLockedSelector.SelectedKeyName, Combat->IsPatternLocked());
		BB->SetValueAsBool(IsInRecoverySelector.SelectedKeyName, Combat->IsInRecovery());
		BB->SetValueAsBool(InExecuteRangeSelector.SelectedKeyName, Combat->IsInExecuteRange(Current, ClosestDistance));
		BB->SetValueAsEnum(CurrentPatternSelector.SelectedKeyName, static_cast<uint8>(Current));
	}
}
