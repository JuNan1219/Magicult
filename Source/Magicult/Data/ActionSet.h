// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpellActionTypes.h"
#include "ActionSet.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UActionSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSpellActionSpec> ActionSets;
};
