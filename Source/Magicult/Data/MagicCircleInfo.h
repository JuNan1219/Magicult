// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MagicCircleInfo.generated.h"

USTRUCT(BlueprintType)
struct FCircleInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MagicElementTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> ElementMaterial;
};

/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicCircleInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	FCircleInfo FindMagicCircleInfoForTag(const FGameplayTag& MagicElementTag, bool bLogNotFound = false) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FCircleInfo> MagicCircleInformation;
};
