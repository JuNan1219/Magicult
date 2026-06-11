// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Magicult/Data/SpellActionTypes.h"
#include "ActionExecutorSubsystem.generated.h"

struct FSpellContext;

/**
 *
 */
UCLASS()
class MAGICULT_API UActionExecutorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	TArray<FSpellTriggerEventData> ExecuteRequest(const TArray<FSpellActionExecutionRequest>& Request);

	// Tag를 ImmunityPolicy에서 조회하여 대상 ASC가 면역 태그를 보유 중이면 true 반환
	bool CheckImmunity(const FGameplayTag& Tag, const FSpellContext& Context) const;

	FSpellActionExecutionResult ExecuteSingleAction(
		const FSpellActionSpec& Action,
		const FSpellContext& Context);

private:
	FSpellActionExecutionResult ExecuteConsumePierce(
		const FSpellConsumePierceActionSpec& ConsumePierceSpec,
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecuteApplyDamage(
		const FSpellContext& Context,
		bool IsRadialDamage = false);

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

	FSpellActionExecutionResult ExecuteInterrupt(
		const FSpellInterruptActionSpec& InterruptSpec,
		const FSpellContext& Context);
};
