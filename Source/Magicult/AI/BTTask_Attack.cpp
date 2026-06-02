// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"


EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    EBTNodeResult::Type Type = Super::ExecuteTask(OwnerComp, NodeMemory);
    if (Type == EBTNodeResult::Failed)
        return EBTNodeResult::Failed;

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        return EBTNodeResult::Failed;
    }

    APawn* Pawn = AICon->GetPawn();
    if (!Pawn)
    {
        return EBTNodeResult::Failed;
    }

    UAbilitySystemComponent* ASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);

    if (!ASC)
    {
        return EBTNodeResult::Failed;
    }

    CachedOwnerComp = &OwnerComp;
    CachedASC = ASC;

    AbilityEndedHandle = ASC->OnAbilityEnded.AddUObject(this, &UBTTask_Attack::OnAbilityEnded);

    FGameplayEventData Payload;
    Payload.EventTag = EventTag;
    Payload.Instigator = Pawn;
    Payload.Target = Pawn;

    const int32 TriggeredCount = ASC->HandleGameplayEvent(EventTag, &Payload);

    if (TriggeredCount > 0)
    {
        // 이 EventTag로 실제 트리거된 GA가 있음
        return EBTNodeResult::InProgress;
    }

    if (AbilityEndedHandle.IsValid())
    {
        ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
        AbilityEndedHandle.Reset();
    }

    // 이 EventTag로 실제 트리거된 GA가 없음
    return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (UAbilitySystemComponent* ASC = CachedASC.Get())
    {
        if (AbilityEndedHandle.IsValid())
        {
            ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
            AbilityEndedHandle.Reset();
        }
    }

    return EBTNodeResult::Aborted;
}

void UBTTask_Attack::OnAbilityEnded(const FAbilityEndedData& EndedData)
{
    if (!EndedData.AbilityThatEnded)
        return;

    if (!WaitAbilityClass)
    {
        return;
    }

    // 여기서 필터링
    if (!EndedData.AbilityThatEnded->IsA(WaitAbilityClass))
        return;

    UAbilitySystemComponent* ASC = CachedASC.Get();
    if (ASC && AbilityEndedHandle.IsValid())
    {
        ASC->OnAbilityEnded.Remove(AbilityEndedHandle);
        AbilityEndedHandle.Reset();
    }

    if (UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get())
    {
        FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
    }
}

