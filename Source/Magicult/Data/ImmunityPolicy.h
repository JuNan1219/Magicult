// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ImmunityPolicy.generated.h"

USTRUCT(BlueprintType)
struct FImmunityRule
{
	GENERATED_BODY()

	// 면역 검사를 받는 태그 (키). Action·Debuff·서브 이펙트 등 어떤 태그든 가능
	// 예: Action.Damage, Debuff.Freeze, Debuff.Effect.Slow
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SubjectTag;

	// 대상 ASC에 이 태그가 있으면 면역으로 판정
	// 예: Immunity.Damage, Immunity.Debuff.Effect.Slow
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag GrantedTag;
};

UCLASS()
class MAGICULT_API UImmunityPolicy : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FImmunityRule> ImmunityRules;

	bool FindImmunityTag(const FGameplayTag& Tag, FGameplayTag& OutImmunityTag) const;
};
