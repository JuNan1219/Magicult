// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultDamageGameplayAbility.h"
#include "MonsterDamageGameplayAbility.generated.h"

/**
 * AI가 사용하는 공격 GameplayAbility의 베이스.
 * 부모의 LocalPredicted를 ServerOnly로 되돌리는 것 외에 동작 차이는 없다.
 */
UCLASS()
class MAGICULT_API UMonsterDamageGameplayAbility : public UMagicultDamageGameplayAbility
{
	GENERATED_BODY()

public:
	UMonsterDamageGameplayAbility();
};
