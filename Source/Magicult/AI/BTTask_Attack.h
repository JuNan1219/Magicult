// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "BTTask_Attack.generated.h"

class UGameplayAbility;

/**
 * 
 */
UCLASS()
class MAGICULT_API UBTTask_Attack : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere)
    FGameplayTag EventTag;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UGameplayAbility> WaitAbilityClass;
private:
    void OnAbilityEnded(const FAbilityEndedData& EndedData);

private:
    TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
    TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

    FDelegateHandle AbilityEndedHandle;
};
