// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
#include "CombatSubsystem.generated.h"

class AMagicultProjectile;
struct FSpellContext;
struct FSpellState;

// 파괴 대기 액터 1건. DeferredFrames는 유예 상한 판정용이며 재등록으로 리셋되지 않는다.
USTRUCT()
struct FPendingDestroyEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> Actor = nullptr;

	int32 DeferredFrames = 0;
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MAGICULT_API UCombatSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Deinitialize() override;
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

	// 큐에 대기 중인 트리거가 참조하는 causer별 개수. 0이 아니면 파괴를 미룬다.
	// TObjectKey는 객체를 붙잡지 않고 파괴 후에도 비교가 성립한다. 수명은 DamageCauserPin이 책임진다.
	TMap<TObjectKey<UObject>, int32> PendingTriggerRefCount;

	void AddTriggerRef(UObject* Causer);
	void ReleaseTriggerRef(UObject* Causer);

	UPROPERTY()
	TArray<FPendingDestroyEntry> PendingDestroyActors;
	UPROPERTY()
	TArray<TObjectPtr<UObject>> PendingReleaseObjects;

	void FlushPendingCleanup();
};
