// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ElementDataTypes.h"
#include "ElementInteractionRules.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UElementInteractionRules : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ElementInteractionRules")
	TArray<FElementReactionRule> ElementReactionRules;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ElementInteractionRules")
	TArray<FDebuffRule> DebuffRules;
};
