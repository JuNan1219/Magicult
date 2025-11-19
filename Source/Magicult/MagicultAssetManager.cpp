// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultAssetManager.h"
#include "MagicultGameplayTags.h"

UMagicultAssetManager& UMagicultAssetManager::Get()
{
	check(GEngine);
	
	UMagicultAssetManager* MagicultAssetManager = Cast<UMagicultAssetManager>(GEngine->AssetManager);
	return *MagicultAssetManager;
}

void UMagicultAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FMagicultGameplayTags::InitializeNativeGameplayTags();
}
