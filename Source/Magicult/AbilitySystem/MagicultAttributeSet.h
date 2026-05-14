// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MagicultAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// 멤버 변수로 이 변수를 사용하지 마시요. Raw pointer GC 위험 있음
USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties() {}

	UPROPERTY()
	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;
	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;
	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;
	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;
	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicultAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMagicultAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 로그 용
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	// 클램프 등을 사용하는 함수, 많은 곳에 접근할 수 있는 강력한 함수
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly ,ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, Health);

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, MaxHealth)
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, Mana)

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, MaxMana)

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxSpeed, Category = "Attributes")
	FGameplayAttributeData MaxSpeed;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, MaxSpeed)

	UFUNCTION()
	void OnRep_MaxSpeed(const FGameplayAttributeData& OldMaxSpeed) const;

	/*
	 * Meta Attributes
	 */

	UPROPERTY()
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, IncomingDamage);

	/*
	 * Meta Attributes End
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RockAffinity, Category = "Primary Attributes")
	FGameplayAttributeData RockAffinity;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, RockAffinity)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FlameAffinity, Category = "Primary Attributes")
	FGameplayAttributeData FlameAffinity;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, FlameAffinity)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IceAffinity, Category = "Primary Attributes")
	FGameplayAttributeData IceAffinity;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, IceAffinity)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WindAffinity, Category = "Primary Attributes")
	FGameplayAttributeData WindAffinity;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, WindAffinity)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LightningAffinity, Category = "Primary Attributes")
	FGameplayAttributeData LightningAffinity;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, LightningAffinity)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageModifier, Category = "Primary Attributes")
	FGameplayAttributeData DamageModifier;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, DamageModifier)

	UFUNCTION()
	void OnRep_RockAffinity(const FGameplayAttributeData& OldRockAffinity) const;

	UFUNCTION()
	void OnRep_FlameAffinity(const FGameplayAttributeData& OldFlameAffinity) const;

	UFUNCTION()
	void OnRep_IceAffinity(const FGameplayAttributeData& OldIceAffinity) const;

	UFUNCTION()
	void OnRep_WindAffinity(const FGameplayAttributeData& OldWindAffinity) const;

	UFUNCTION()
	void OnRep_LightningAffinity(const FGameplayAttributeData& OldLightningAffinity) const;

	UFUNCTION()
	void OnRep_DamageModifier(const FGameplayAttributeData& OldDamageModifier) const;


	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RockResistance, Category = "Secondary Attributes")
	FGameplayAttributeData RockResistance;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, RockResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FlameResistance, Category = "Secondary Attributes")
	FGameplayAttributeData FlameResistance;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, FlameResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IceResistance, Category = "Secondary Attributes")
	FGameplayAttributeData IceResistance;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, IceResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WindResistance, Category = "Secondary Attributes")
	FGameplayAttributeData WindResistance;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, WindResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LightningResistance, Category = "Secondary Attributes")
	FGameplayAttributeData LightningResistance;
	ATTRIBUTE_ACCESSORS(UMagicultAttributeSet, LightningResistance);

	UFUNCTION()
	void OnRep_RockResistance(const FGameplayAttributeData& OldRockResistance) const;

	UFUNCTION()
	void OnRep_FlameResistance(const FGameplayAttributeData& OldFlameResistance) const;

	UFUNCTION()
	void OnRep_IceResistance(const FGameplayAttributeData& OldIceResistance) const;

	UFUNCTION()
	void OnRep_WindResistance(const FGameplayAttributeData& OldWindResistance) const;

	UFUNCTION()
	void OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const;

private:
	void HandleIncomingDamage(const FEffectProperties& Props);
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;

};
