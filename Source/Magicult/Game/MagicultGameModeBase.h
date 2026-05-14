// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MagicultGameModeBase.generated.h"

class UMagicInfo;
class UTagToData;
class UElementInteractionRules;
class UProjectileData;
class UCharacterClassInfo;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UMagicInfo> MagicInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UTagToData> TagToData;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UElementInteractionRules> ElementInteractionRules;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UProjectileData> ProjectileData;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage")
	TSubclassOf<UGameplayEffect> DefaultDamageGameplayEffect;
};
