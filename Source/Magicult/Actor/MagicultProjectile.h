// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magicult/MagicultAbilityTypes.h"
#include "Magicult/Data/ProjectileTypes.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/Interface/ActionSourceInterface.h"
#include "Magicult/Interface/AttackContextProvider.h"
#include "Magicult/Interface/CombatRuntimeObject.h"
#include "Magicult/Component/CollisionGuardComponent.h"
#include "MagicultProjectile.generated.h"

class UProjectileMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;

USTRUCT()
struct FProjectileClientInitData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bValid = false;

	UPROPERTY()
	FVector LaunchDirection = FVector::ForwardVector;

	UPROPERTY()
	float InitialSpeed = 0.f;

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> ProjectileNiagara = nullptr;

	UPROPERTY()
	FSpellColliderSpec ProjectileCollider;

	UPROPERTY()
	ESpellCollisionEventPolicy ColliderEventPolicy = ESpellCollisionEventPolicy::None;

	UPROPERTY()
	float Bounciness = 1.f;

	UPROPERTY()
	float SizeScale = 1.0f;

	UPROPERTY()
	float GravityScale = 1.0f;

	UPROPERTY()
	float LifeSpanMultiplier = 1.0f;

	UPROPERTY()
	bool bTracking = false;

	UPROPERTY()
	float TrackingSearchRadius = 1000.0f;

	UPROPERTY()
	float HomingAccelerationMagnitude = 3000.0f;
};

UCLASS(Abstract)
class MAGICULT_API AMagicultProjectile : public AActor, public IActionSourceInterface, public IAttackContextProvider, public ICombatRuntimeObject
{
	GENERATED_BODY()
public:
	AMagicultProjectile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetProjectileSpawnData(const FSpellContext& InSpellContext, const FResolvedSpellProjectileResult& InResolvedSpellProjectileResult);
	virtual void InitializeProjectile();
	virtual UPrimitiveComponent* GetCollider();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCollisionGuardComponent> CollisionGuard;

	UPROPERTY(Replicated)
	FSpellContext SpellContext;

	FResolvedSpellProjectileResult ResolvedProjectileResult;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileClientInitData)
	FProjectileClientInitData ProjectileClientInitData;

	UFUNCTION()
	void OnRep_ProjectileClientInitData();



	UFUNCTION()
	void OnRep_TriggerActions();

	virtual void ApplyColliderSpec(const FSpellColliderSpec& ColliderSpec);

	/*
	* Interface
	*/
	virtual const TArray<FSpellTriggerActionBinding>& GetTriggerActionBindings() const override;

	virtual FAttackSpec GetAttackSpec() const override;
	virtual FVector GetActorVelocity() const override;

	virtual void PrepareCleanup_Implementation() override;
	/*
	* Interface end
	*/

	void ProcessOverlapRehit(float DeltaTime);
protected:
	virtual void BeginPlay() override;

	virtual void LifeSpanExpired() override;
	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAttackSpec AttackSpec;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	float LifeSpan = 3.f;

	UPROPERTY(BlueprintReadOnly)
	float RehitCooldown = 0.5f;

	UPROPERTY(ReplicatedUsing = OnRep_TriggerActions, BlueprintReadOnly)
	TArray<FSpellTriggerActionBinding> TriggerActions;
private:
	bool bProjectileInitialized = false;
	bool IsValidImpact(AActor* SourceAvatarActor, AActor* OtherActor) const;

	void BindCollisionPolicy(ESpellCollisionEventPolicy Policy);
	void ConfigureCollisionForPolicy(ESpellCollisionEventPolicy Policy);

	UFUNCTION()
	void HandleGuardedCollision(const FGuardedCollisionEvent& CollisionEvent);

	UFUNCTION()
	void HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	bool bExpired = false;
	void Expire();
	virtual void OnExpired();

	void UnbindCollisionEvents();

	UPROPERTY()
	bool bCleanupPrepared = false;
};
