// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ElementDataTypes.h"
#include "ScalableFloat.h"
#include "TagToData.generated.h"

class UNiagaraSystem;

/**
 * 
 */
UCLASS()
class MAGICULT_API UTagToData : public UDataAsset
{
	GENERATED_BODY()

public:
    // 수치형 (speed, direction)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TMap<FGameplayTag, float> FloatValues;

    // 클래스 참조형
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TMap<FGameplayTag, TSubclassOf<AActor>> ClassValues;

    // 나이아가라 데이터
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TMap<FGameplayTag, TObjectPtr<UNiagaraSystem>> NiagaraValues;

    // 레벨별 데이터
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TMap<FGameplayTag, FScalableFloat> ScalableFloatValues;
};
