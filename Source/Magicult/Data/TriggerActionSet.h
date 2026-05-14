// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpellActionTypes.h"
#include "TriggerActionSet.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UTriggerActionSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSpellTriggerActionBinding> Bindings;
};
