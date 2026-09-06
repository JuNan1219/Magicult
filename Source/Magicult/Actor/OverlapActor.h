// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/Interface/ActionSourceInterface.h"
#include "Magicult/Interface/AttackContextProvider.h"
#include "Magicult/Interface/CombatRuntimeObject.h"
#include "Magicult/Component/CollisionGuardComponent.h"
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

	void SetOverlapConfig(const UOverlapDamageConfig* InConfig);

	UFUNCTION(BlueprintCallable)
	void SetAttackSpec(const FAttackSpec& InAttackSpec);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCollisionGuardComponent> CollisionGuard;

	UPROPERTY(Replicated)
	FSpellContext SpellContext;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UPrimitiveComponent* GetCollider();

	virtual UPrimitiveComponent* GetCollider_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = false;

	UFUNCTION(BlueprintCallable)
	void DrawDebug(bool bEnabled);

	UFUNCTION(BlueprintCallable)
	void EnableOverlap();
	UFUNCTION(BlueprintCallable)
	void DisableOverlap();

	// 콜라이더 on/off는 컴포넌트 상태라 그 자체론 복제되지 않는다.
	// 서버가 여기 세팅한 값을 OnRep으로 각 클라가 받아 자기 콜라이더에 동일하게 반영한다.
	UPROPERTY(ReplicatedUsing = OnRep_bOverlapActive)
	bool bOverlapActive = false;

	UFUNCTION()
	void OnRep_bOverlapActive();

	void ApplyOverlapActive();
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

	/**
	 * Config를 적용하는 확장 지점. beginplay나 config 세팅 시점에 이걸 호출한다.
	 * BP에서 override하면 콜라이더 크기 등 자기 컴포넌트 초기화를 추가할 수 있고,
	 * 이때 CollisionGuard 반영을 위해 반드시 Parent를 호출해야 한다.
	 * 스폰 직후/BeginPlay 이전이든 이후든 안전하게 호출 가능(타이밍은 내부에서 처리).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Overlap")
	void ApplyOverlapConfig();

	void ApplyOverlapConfig_Implementation();

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const UOverlapDamageConfig> OverlapConfig;
private:
	UFUNCTION()
	void HandleGuardedCollision(const FGuardedCollisionEvent& CollisionEvent);
	void UnbindOverlapGuard();

	UPROPERTY()
	bool bExpired = false;

	UPROPERTY()
	bool bCleanupPrepared = false;

	
};
