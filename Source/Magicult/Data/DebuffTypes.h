#pragma once

#include "GameplayTagContainer.h"
#include "DebuffTypes.generated.h"

class UGameplayEffect;

UENUM(BlueprintType)
enum class EDebuffEffectType : uint8
{
    None,
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

    // 이 서브 이펙트를 식별하는 태그. ImmunityPolicy 키로 사용된다.
    // 예: Debuff.Effect.Slow, Debuff.Effect.PeriodicDamage
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag Tag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EDebuffEffectType Type = EDebuffEffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> EffectClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Type==EDebuffEffectType::Slow", EditConditionHides))
    FSlowEffectData SlowData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "Type==EDebuffEffectType::PeriodicDamage", EditConditionHides))
    FPeriodicDamageEffectData DamageData;
};