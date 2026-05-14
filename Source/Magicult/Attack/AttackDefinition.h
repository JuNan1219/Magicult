#pragma once

#include "GameplayTagContainer.h"
#include "Magicult/Data/AttackSpec.h"
#include "AttackDefinition.generated.h"

class UAnimMontage;
class UAttackInstance;
class UAttackInstanceConfig;

USTRUCT(BlueprintType)
struct FAttackDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FAttackSpec AttackSpec;

    UPROPERTY(EditAnywhere)
    bool bOverrideAttackSpec = false;

    UPROPERTY(EditAnywhere)
    TObjectPtr<UAnimMontage> Montage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag SocketTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<USoundBase> ImpactSound = nullptr;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UAttackInstance> AttackInstanceClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAttackInstanceConfig> InstanceConfig = nullptr;

    UPROPERTY(EditAnywhere)
    FGameplayTagContainer AttackTags;
};

USTRUCT(BlueprintType)
struct FTaggedAttackDefinitions
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag TriggerTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAttackDefinition> Attacks;
};