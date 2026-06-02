// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Magicult/Interface/MagicInterface.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/Attack/AttackDefinition.h"
#include "MagicultCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UMagicComponent;
class USkeletalMeshComponent;
class UMaterialInstance;
class UMaterialInstanceDynamic;
class USkeletalMeshSocket;
struct FOnAttributeChangeData;

UCLASS(Abstract)
class MAGICULT_API AMagicultCharacterBase : public ACharacter, public IAbilitySystemInterface, public IMagicInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AMagicultCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/** Combat Interface */
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	virtual FTransform GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag) override;
	virtual bool AttachActorToSocket_Implementation(const FGameplayTag& SocketTag, AActor* Target) override;
	virtual void Die() override;
	virtual bool IsDead_Implementation() const override;
	virtual const TArray<FAttackDefinition> GetAttackDefinitions_Implementation(const FGameplayTag& InputTag) override;
	/** end Combat Interface */
	/** Magic Interface */
	virtual UMagicComponent* GetMagicComponent_Implementation() override;
	/** End Magic Interface */

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FTaggedAttackDefinitions> TaggedAttackDefinitions;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName HeadSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName MagicSocketName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
	TObjectPtr<UMagicComponent> MagicComponent;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	virtual void InitializeDefaultAttributes() const;

	void AddCharacterAbilities();

	/*
	* Dissolve Effects
	*/
	void Dissolve();

	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	UPROPERTY(BlueprintReadOnly)
	bool bDead = false;

	UFUNCTION(BlueprintNativeEvent)
	void ApplyMaxSpeed(float InMaxSpeed);
	virtual void ApplyMaxSpeed_Implementation(float InMaxSpeed);

private:
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	void OnMaxSpeedChanged(const FOnAttributeChangeData& Data);
	void ApplyCurrentMaxSpeed();
};
