// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Magicult/Data/SpellActionTypes.h"
#include "Magicult/Data/ActionExecutionConfig.h"
#include "ActionExecutorSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UActionExecutorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	TArray<FSpellTriggerEventData> ExecuteRequest(const TArray<FSpellActionExecutionRequest>& Request);

	const UActionExecutionConfig* GetActionExecutionConfig() const;
	bool CanPassRequiredTags(const UAbilitySystemComponent* ASC, const FGameplayTagContainer& RequiredTags) const;
	bool CanPassBlockedTags(const UAbilitySystemComponent* ASC, const FGameplayTagContainer& BlockedTags) const;
	bool CanExecuteAction(ESpellActionType ActionType, const FSpellContext& Context) const;

	FSpellActionExecutionResult ExecuteSingleAction(
		const FSpellActionSpec& Action,
		const FSpellContext& Context);

private:
	FSpellActionExecutionResult ExecuteConsumePierce(
		const FSpellConsumePierceActionSpec& ActConsumePierceSpecion,
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecuteApplyDamage(
		const FSpellContext& Context
		, bool IsRadialDamage = false);

	FSpellActionExecutionResult ExecuteRadialDamage(
		const FSpellRadialDamageActionSpec& RadialDamageSpec,
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecuteResolveElementInteraction(
		const FSpellApplyElementActionSpec& ApplyElementSpec,
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecuteApplyDebuff(
		const FSpellDebuffActionSpec& DebuffSpec,
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecuteSpawnActor(
		const FSpellSpawnActorActionSpec& SpawnActorSpec,
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecuteGameplayEffect(
		const FSpellApplyGEActionSpec& ApplyGESpec,
		const FSpellContext& Context);
	
	FSpellActionExecutionResult ExecuteEmitNiagara(
		const FSpellEmitNiagaraActionSpec& EmitNiagaraSpec,
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecutePlayCue(
		const FSpellPlayCueActionSpec& PlayCueSpec,
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecuteKnockback(
		const FSpellKnockbackActionSpec& KnockbackSpec, 
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecuteDestroyActor(
		const FSpellContext& Context);

	UPROPERTY(Transient)
	mutable TObjectPtr<UActionExecutionConfig> ActionExecutionConfig;
};
