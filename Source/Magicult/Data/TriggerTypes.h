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

	// 체인 반응 깊이. 플레이어/AI 직접 발생 이벤트=0, 파생 이벤트=부모+1.
	// ProcessTriggerQueue가 이 값으로 무한 체인을 차단한다.
	int32 ChainDepth = 0;
};

/*
* Trigger and Action 매핑 데이터 끝
*/