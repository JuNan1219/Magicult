#pragma once

#include "DebuffTypes.generated.h"

class UGameplayEffect;

UENUM(BlueprintType)
enum class EDebuffEffectType : uint8
{
    Slow,
    PeriodicDamage,
};

USTRUCT(BlueprintType)
struct FPeriodicDamageEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float DamagePerTick = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float TickInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Duration = 5.0f;
};
USTRUCT(BlueprintType)
struct FSlowEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MoveSpeedMultiplier = 0.7f; // 0.7 = 30% 감속

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Duration = 3.0f;
};
USTRUCT(BlueprintType)
struct FDebuffEffectEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EDebuffEffectType Type;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> EffectClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Type==EDebuffEffectType::Slow"))
    FSlowEffectData SlowData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Type==EDebuffEffectType::PeriodicDamage"))
    FPeriodicDamageEffectData DamageData;
};