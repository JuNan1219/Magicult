// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultAssetManager.h"
#include "AbilitySystemGlobals.h"
#include "MagicultGameplayTags.h"

UMagicultAssetManager& UMagicultAssetManager::Get()
{
    UMagicultAssetManager* Singleton = Cast<UMagicultAssetManager>(GEngine->AssetManager);

    if (Singleton)
    {
        return *Singleton;
    }

    UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManagerClass in DefaultEngine.ini."));

    return *NewObject<UMagicultAssetManager>();
}

void UMagicultAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	FMagicultGameplayTags::InitializeNativeGameplayTags();

	// TargetData(GameplayAbilityTargetData) 사용을 위해 필요
	UAbilitySystemGlobals::Get().InitGlobalData();
}
