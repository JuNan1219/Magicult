// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Magicult/Data/ProjectileTypes.h"
#include "ProjectileData.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UProjectileData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "ProjectileData")
	TArray<FSpellElementSpec> ElementData;

	UPROPERTY(EditDefaultsOnly, Category = "ProjectileData")
	TArray<FSpellProjectileFeatureSpec> AmmoTypeData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TArray<FSpellProjectileClassEntry> ProjectileBaseClasses;
};
