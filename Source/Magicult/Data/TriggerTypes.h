#pragma once

#include "SpellTypes.h"
#include "TriggerTypes.generated.h"

class AMagicultProjectile;

/*
* Trigger and Action 매핑 데이터
*/
USTRUCT(BlueprintType)
struct FSpellTriggerSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag TriggerTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger")
	FGameplayTagContainer TriggerContextTags;
};

USTRUCT(BlueprintType)
struct FSpellTriggerEventData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag TriggerTag;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer TriggerContextTags;

	UPROPERTY(BlueprintReadWrite)
	FSpellContext SpellContext;
};

/*
* Trigger and Action 매핑 데이터 끝
*/