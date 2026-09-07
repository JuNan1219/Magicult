// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Magicult/Data/SpellActionTypes.h"
#include "ActionExecutorSubsystem.generated.h"

struct FSpellContext;
class AActor;

/**
 *
 */
UCLASS()
class MAGICULT_API UActionExecutorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	TArray<FSpellTriggerEventData> ExecuteRequest(const TArray<FSpellActionExecutionRequest>& Request, int32& OutActionsExecuted);

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

	FSpellActionExecutionResult ExecuteSpawnAttackInstance(
		const FSpellSpawnAttackInstanceActionSpec& Spec,
		const FSpellContext& Context);

	FSpellActionExecutionResult ExecuteGameplayEffect(
		const FSpellApplyGEActionSpec& ApplyGESpec,
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

	// --- Policy 기반 위치 해석 ---
	AActor* ResolveActorByPolicy(ESpellActionTargetPolicy Policy, const FSpellContext& Context) const;

	// Policy를 Transform으로 해석. 해석 실패 시 Identity 반환하고 bOutResolved=false.
	FTransform ResolveTransformByPolicy(ESpellActionTargetPolicy Policy, const FSpellContext& Context, FGameplayTag SocketTag, bool* bOutResolved = nullptr) const;

	// Policy를 위치로 해석. 해석 실패 시 fallback(DamageCauser→Source)으로 원점 사고 방지.
	FVector ResolveOriginByPolicy(ESpellActionTargetPolicy Policy, const FSpellContext& Context, FGameplayTag SocketTag) const;
};
