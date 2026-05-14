// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpellActionTypes.h"
#include "ActionExecutionConfig.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UActionExecutionConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FActionConfig> ActionConfigs;

    bool FindActionConfig(ESpellActionType ActionType, FActionConfig& OutActionConfig) const;
};
