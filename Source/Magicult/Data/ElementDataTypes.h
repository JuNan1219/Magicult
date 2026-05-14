#pragma once

#include "GameplayTagContainer.h"
#include "Magicult/Data/SpellActionTypes.h"
#include "ElementDataTypes.generated.h"

class UGameplayEffect;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FElementReactionRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag IncomingElementTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag ExistingTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag ReactionTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bRemoveExistingTag = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bApplyDebuff = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FSpellActionSpec> InteractionActions;

    // 정렬용
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct FDebuffRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag IncomingElementTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag DebuffTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FDebuffEffectEntry> EffectSpecs;
};

USTRUCT(BlueprintType)
struct FElementInteractionResolveResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TArray<FGameplayTag> RemoveTags;

    UPROPERTY(BlueprintReadOnly)
    TArray<FSpellActionExecutionRequest> InteractionActionRequests;

    UPROPERTY(BlueprintReadOnly)
    TArray<FSpellActionExecutionRequest> DebuffActionRequests;
};