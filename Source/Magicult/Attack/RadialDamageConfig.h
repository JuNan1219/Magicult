// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackInstanceConfig.h"
#include "RadialDamageConfig.generated.h"

/**
 * URadialAttackInstance 초기화용 Config.
 * 구 FSpellRadialDamageActionSpec의 필드를 1:1로 옮겨, 후속 데이터 이관 시 매핑이 자명하도록 한다.
 * 데미지/넉백은 여기 두지 않는다 — RadialAttackInstance가 발행하는 Trigger.Hit에 걸린
 * 바인딩(ApplyDamage / Knockback 등)이 처리한다. enemies-only는 AttackSpec.bTargetEnemiesOnly 사용.
 */
UCLASS()
class MAGICULT_API URadialDamageConfig : public UAttackInstanceConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Radial")
	float Radius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Radial")
	bool bIgnoreOwner = false;
};
