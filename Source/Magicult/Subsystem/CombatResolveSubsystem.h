// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Magicult/Data/ElementInteractionRules.h"
#include "Magicult/MagicultAbilityTypes.h"
#include "Magicult/Data/ProjectileTypes.h"
#include "Magicult/Data/SpellTypes.h"
#include "CombatResolveSubsystem.generated.h"

struct FSpellState;

/**
 * 
 */
UCLASS()
class MAGICULT_API UCombatResolveSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Combat|Resolve")
	FResolvedSpellProjectileResult ResolveProjectile(const FSpellContext& InSpellContext, const FSpellState& InSpellState) const;

	FResolvedSpellProjectileSpec ResolveProjectileSpec(const FSpellContext& InSpellContext, const FSpellState& InSpellState, FSpellRuntimeState& OutRuntimeState) const;

	UFUNCTION(BlueprintCallable, Category = "Combat|Resolve")
	FElementInteractionResolveResult ResolveIncomingElement(const FSpellContext& InSpellContext, const FGameplayTag& IncomingElementTag);
	bool ResolveElementInteraction(const FSpellContext& InSpellContext,const FGameplayTag& IncomingElementTag, FElementInteractionResolveResult& CombatResolveResult);
	void ResolveElementDebuff(const FSpellContext& InSpellContext,const FGameplayTag& IncomingElementTag, FElementInteractionResolveResult& CombatResolveResult);
protected:

};
