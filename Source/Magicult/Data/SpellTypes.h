#pragma once

#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "SpellTypes.generated.h"

class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FSpellContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> SourceActor;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> DamageCauser;

	UPROPERTY(BlueprintReadWrite)
	FHitResult HitResult;

	UPROPERTY(BlueprintReadWrite)
	FGameplayEffectSpecHandle DamageSpecHandle;

	UPROPERTY(BlueprintReadWrite)
	bool bHasHitResult = false;
};


