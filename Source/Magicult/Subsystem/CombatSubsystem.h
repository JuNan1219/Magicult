// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CombatSubsystem.generated.h"

class AMagicultProjectile;
struct FSpellContext;
struct FSpellState;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MAGICULT_API UCombatSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	AMagicultProjectile* SpawnProjectile(FSpellContext& Context, const FSpellState& SpellState, const FTransform& SpawnTransform, float BaseDamage);
	
	UFUNCTION(BlueprintCallable)
	void Enqueue(FSpellTriggerEventData NewTriggerEventData);

	void RegisterPendingCleanupObject(UObject* RuntimeObject);
	
private:
	TQueue<FSpellTriggerEventData> TriggerQueue;

	// 모든 enqueue의 단일 관문. DamageCauserPin이 비어있으면 SpellContext.DamageCauser로 채운다.
	void EnqueueInternal(FSpellTriggerEventData& TriggerEventData);

	void ProcessTriggerQueue();

	bool IsServer();

	UPROPERTY()
	TArray<TObjectPtr<AActor>> PendingDestroyActors;
	UPROPERTY()
	TArray<TObjectPtr<UObject>> PendingReleaseObjects;

	void FlushPendingCleanup();
};
