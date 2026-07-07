// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_ExecuteSelectedPattern.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "BossCombatComponent.h"

UBTTask_ExecuteSelectedPattern::UBTTask_ExecuteSelectedPattern()
{
	NodeName = TEXT("Execute Selected Pattern");

	// 이 Task는 실행 상태(CachedASC/AbilityEndedHandle/ExecutingPattern 등)를 멤버 변수로 들고 있다.
	// BT Task 노드는 기본적으로 트리 애셋당 하나의 객체를 모든 AI가 공유하므로,
	// 인스턴스 분리(bCreateNodeInstance) 없이는 AI가 여러 마리거나 재진입 시 상태가 서로 덮어써진다.
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_ExecuteSelectedPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UBossCombatComponent* Combat = Pawn->FindComponentByClass<UBossCombatComponent>();
	if (!Combat)
	{
		return EBTNodeResult::Failed;
	}

	ExecutingPattern = Combat->GetCurrentPattern();
	const FBossAttackPatternData* Data = Combat->FindPatternData(ExecutingPattern);
	if (!Data || !Data->AttackEventTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}
	ExecutingAbilityClass = Data->AbilityClass;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(TargetActorSelector.SelectedKeyName)) : nullptr;

	// 공격 실행 전 반드시 이동을 멈춘다. 방향 정렬은 Motion Warping이 담당하므로
	// SetFocus로 공격 내내 타겟을 계속 쫓아 도는 것은 하지 않는다(커밋된 공격 궤적을 방해함).
	AICon->StopMovement();

	// 재진입 방어: 혹시 이전 실행의 델리게이트가 안 풀렸으면 새로 걸기 전에 정리한다.
	CleanupDelegate();

	CachedOwnerComp = &OwnerComp;
	CachedASC = ASC;
	AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &UBTTask_ExecuteSelectedPattern::OnAbilityEnded);

	FGameplayEventData Payload;
	Payload.EventTag = Data->AttackEventTag;
	Payload.Instigator = Pawn;
	Payload.Target = Target;

	const int32 TriggeredCount = ASC->HandleGameplayEvent(Data->AttackEventTag, &Payload);
	if (TriggeredCount > 0)
	{
		return EBTNodeResult::InProgress;
	}

	// 이 태그로 트리거된 GA가 없음.
	CleanupDelegate();
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_ExecuteSelectedPattern::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CleanupDelegate();
	return EBTNodeResult::Aborted;
}

void UBTTask_ExecuteSelectedPattern::OnAbilityEnded(const FAbilityEndedData& EndedData)
{
	// 실행 중인 패턴의 AbilityClass가 지정됐으면 해당 클래스 GA 종료만 인정(HitReact 등 오탐 방지).
	if (ExecutingAbilityClass)
	{
		if (!EndedData.AbilityThatEnded || !EndedData.AbilityThatEnded->IsA(ExecutingAbilityClass))
		{
			return;
		}
	}

	CleanupDelegate();

	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	if (!OwnerComp)
	{
		return;
	}

	// 상태 전이: 실행 완료 → 쿨다운/Recovery 시작, 잠금 해제.
	if (AAIController* AICon = OwnerComp->GetAIOwner())
	{
		if (APawn* Pawn = AICon->GetPawn())
		{
			if (UBossCombatComponent* Combat = Pawn->FindComponentByClass<UBossCombatComponent>())
			{
				Combat->OnPatternExecuted(ExecutingPattern);

				// 주의: 이 함수는 BT 틱 스택 밖(GA 종료 콜백)에서 불리므로,
				// UBTFunctionLibrary 대신 캐싱된 OwnerComp의 Blackboard를 직접 사용한다.
				// SetValueAsBool/Enum은 FName 오버로드만 있으므로 SelectedKeyName을 쓴다.
				if (UBlackboardComponent* BB = OwnerComp->GetBlackboardComponent())
				{
					BB->SetValueAsBool(PatternLockedSelector.SelectedKeyName, false);
					BB->SetValueAsBool(IsInRecoverySelector.SelectedKeyName, Combat->IsInRecovery());
					BB->SetValueAsEnum(CurrentPatternSelector.SelectedKeyName, static_cast<uint8>(EBossPattern::None));
				}
			}
		}
	}

	FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
}

void UBTTask_ExecuteSelectedPattern::CleanupDelegate()
{
	if (UAbilitySystemComponent* ASC = CachedASC.Get())
	{
		if (AbilityEndedHandle.IsValid())
		{
			ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
		}
	}
	AbilityEndedHandle.Reset();
}
