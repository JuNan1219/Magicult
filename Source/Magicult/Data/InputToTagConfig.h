// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputToTagConfig.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FInputToSelect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SelectedTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;
};

/**
 * 
 */
UCLASS()
class MAGICULT_API UInputToTagConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	const FInputToSelect FindTagToInput(const FGameplayTag& InputTag, bool& bFound) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FInputToSelect> InputToSelects;
};
