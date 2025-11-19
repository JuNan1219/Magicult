// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "MagicultAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicultAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	static UMagicultAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};
