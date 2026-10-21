#pragma once

#include "SpellTypes.h"
#include "UObject/StrongObjectPtr.h"
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

	// causer 수명 pin. 트리거가 큐에 있는 동안 DamageCauser(UObject)를 GC로부터 붙잡는다.
	// UPROPERTY 불가(BlueprintType USTRUCT + TStrongObjectPtr). TQueue는 비-UPROPERTY 컨테이너라
	// TObjectPtr는 GC 추적이 안 되므로 TStrongObjectPtr만 유효하다.
	// EnqueueInternal이 SpellContext.DamageCauser로부터 자동으로 채운다.
	// 파괴 유예 참조 카운트의 키이기도 하다 - 파괴 후 nullptr이 되는 DamageCauser와 달리
	// 강참조라 감소 시점에도 같은 포인터를 준다.
	TStrongObjectPtr<UObject> DamageCauserPin;
};

/*
* Trigger and Action 매핑 데이터 끝
*/