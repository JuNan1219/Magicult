// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SpellActionTypes.h"
#include "InterruptPolicy.generated.h"

USTRUCT(BlueprintType)
struct FInterruptRule
{
	GENERATED_BODY()

	// 이 ResultTag가 발생했을 때 규칙 적용
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag TriggerActionTag;

	// 실행할 인터럽트 스펙 — CancelTags에 매칭되는 어빌리티가 없으면 no-op
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSpellInterruptActionSpec InterruptSpec;
};

UCLASS()
class MAGICULT_API UInterruptPolicy : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FInterruptRule> InterruptRules;
};
