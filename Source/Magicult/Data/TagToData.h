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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TMap<FGameplayTag, TSubclassOf<AActor>> ClassValues;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TMap<FGameplayTag, TObjectPtr<UNiagaraSystem>> NiagaraValues;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TMap<FGameplayTag, FScalableFloat> ScalableFloatValues;
};
