#pragma once

#include "Magicult/Actor/OverlapActor.h"
#include "Magicult/Data/AttackSpec.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/Attack/OverlapDamageConfig.h"

#include "OverlapActorSpawnParam.generated.h"

USTRUCT(BlueprintType)
struct FOverlapActorSpawnParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSpellContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AOverlapActor> OverlapActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform SpawnTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAttackSpec AttackSpec;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOverrideAttackSpec = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UOverlapDamageConfig* CollisionConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> Owner = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<APawn> Instigator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSpellTriggerActionBinding> ParentTriggerActionBindings;
};