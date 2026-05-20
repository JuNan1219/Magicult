#pragma once

#include "GameplayEffectTypes.h"
#include "MagicultAbilityTypes.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FAttackContext
{
	GENERATED_BODY()

	FAttackContext() {}

	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FVector DamageCauserVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag ElementType = FGameplayTag();

	UPROPERTY(BlueprintReadWrite)
	int32 ElementLevel = 1;

	UPROPERTY(BlueprintReadWrite)
	float ElementRatio = 0.f;

	UPROPERTY(BlueprintReadWrite)
	bool bUseVelocityDamage = false;

	UPROPERTY(BlueprintReadWrite)
	float DamageModifier = 1.0f;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

USTRUCT(BlueprintType)
struct FAttackApplyParams
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> DamageCauser;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	FAttackContext AttackContext;
};

USTRUCT(BlueprintType)
struct FMagicultGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetScriptStruct() const
	{
		return FMagicultGameplayEffectContext::StaticStruct();
	}

	virtual FMagicultGameplayEffectContext* Duplicate() const
	{
		FMagicultGameplayEffectContext* NewContext = new FMagicultGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		NewContext->AttackContext = AttackContext;

		return NewContext;
	}
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	UPROPERTY()
	FAttackContext AttackContext;
};

template<>
struct TStructOpsTypeTraits< FMagicultGameplayEffectContext > : public TStructOpsTypeTraitsBase2< FMagicultGameplayEffectContext >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true		// Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};

template<>
struct TStructOpsTypeTraits< FAttackContext > : public TStructOpsTypeTraitsBase2< FAttackContext >
{
	enum
	{
		WithNetSerializer = true,
	};
};