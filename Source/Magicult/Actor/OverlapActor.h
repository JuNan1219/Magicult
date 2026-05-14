// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/Interface/ActionSourceInterface.h"
#include "Magicult/Interface/AttackContextProvider.h"
#include "Magicult/Interface/CombatRuntimeObject.h"
#include "Magicult/Component/OverlapTriggerGuardComponent.h"
#include "OverlapActor.generated.h"

class USphereComponent;
class UOverlapDamageConfig;

UCLASS(BlueprintType)
class MAGICULT_API AOverlapActor : public AActor, public IActionSourceInterface, public IAttackContextProvider, public ICombatRuntimeObject
{
	GENERATED_BODY()

public:
	AOverlapActor();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	virtual void Initialize(const FSpellContext& InContext, const TArray<FSpellTriggerActionBinding>& ParentTriggerActionBindings);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InitializeWithConfig(const FSpellContext& InContext,const UOverlapDamageConfig* Config, const TArray<FSpellTriggerActionBinding>& ParentTriggerActionBindings, const FAttackSpec& InAttackSpec, bool bOverrideAttackSpec = false);
	virtual void InitializeWithConfig_Implementation(const FSpellContext& InContext,const UOverlapDamageConfig* Config, const TArray<FSpellTriggerActionBinding>& ParentTriggerActionBindings, const FAttackSpec& InAttackSpec, bool bOverrideAttackSpec = false);

	void Expire();
	virtual void OnExpired();

	UFUNCTION(BlueprintCallable)
	void SetAttackSpec(const FAttackSpec& InAttackSpec);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UOverlapTriggerGuardComponent> OverlapGuard;

	UPROPERTY(Replicated)
	FSpellContext SpellContext;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UPrimitiveComponent* GetCollider();

	virtual UPrimitiveComponent* GetCollider_Implementation();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DrawDebug();

	/** 오버랩 활성/비활성 */
	UFUNCTION(BlueprintCallable)
	void EnableOverlap();
	UFUNCTION(BlueprintCallable)
	void DisableOverlap();
	/*
	* Interface
	*/
	
	virtual const TArray<FSpellTriggerActionBinding>& GetTriggerActionBindings() const override;

	virtual FAttackSpec GetAttackSpec() const override;
	virtual FVector GetActorVelocity() const override;
	/*
	* Interface end
	*/

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAttackSpec AttackSpec;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	float LifeSpan = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float RehitCooldown = 0.5f;

	virtual void LifeSpanExpired() override;
	virtual void Destroyed() override;

	virtual void PrepareCleanup_Implementation() override;

	// Blueprint에서 설정하는 바인딩 원본
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly)
	TArray<FSpellTriggerActionBinding> TriggerActionBindings;
private:
	UFUNCTION()
	void HandleGuardedOverlap(const FGuardedOverlapEvent& OverlapEvent);
	void UnbindOverlapGuard();

	UPROPERTY()
	bool bExpired = false;

	UPROPERTY()
	bool bCleanupPrepared = false;
};
