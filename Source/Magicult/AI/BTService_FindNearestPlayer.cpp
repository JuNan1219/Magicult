// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_FindNearestPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Magicult/Character/MagicultEnemy.h"

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* OwningPawn = AIOwner->GetPawn();
	if (!IsValid(OwningPawn)) return;

	const AMagicultEnemy* OwningEnemy = Cast<AMagicultEnemy>(OwningPawn);
	const float MaxDetectRange = OwningEnemy ? OwningEnemy->MaxDetectRange : 0.f;

	// 파벌 태그 기반으로 적대 대상 탐색. ICombatInterface 구현체 전체에서 IsNotFriend로 필터링한다.
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
		if (MaxDetectRange > 0.f && Distance > MaxDetectRange) continue;
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestActor = Actor;
		}
	}
	UBTFunctionLibrary::SetBlackboardValueAsObject(this, TargetToFollowSelector, ClosestActor);
	UBTFunctionLibrary::SetBlackboardValueAsFloat(this, DistanceToTargetSelector, ClosestDistance);
}
