// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackInstance.h"
#include "SpawnProjectileAttackInstance.generated.h"

struct FGameplayTag;
struct FSpellState;
struct FSpellContext;
struct FResolvedSpellProjectileResult;

/**
 * 
 */
UCLASS()
class MAGICULT_API USpawnProjectileAttackInstance : public UAttackInstance
{
	GENERATED_BODY()
public:
	virtual void HandleEventTag_Implementation(const FGameplayTag& EventTag);
private:
	bool ConvertTagsToProjectileData(const FSpellState& CurrentSpellState, FSpellContext& OutSpellContext, FResolvedSpellProjectileResult& OutResolvedSpellProjectileResult);
	bool CheckSpellState(const FSpellState& CurrentSpellState);

	void SpawnMagicProjectile(const FSpellState& CurrentSpellState);
};
