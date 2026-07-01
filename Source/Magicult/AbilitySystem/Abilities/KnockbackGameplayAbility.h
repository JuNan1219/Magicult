// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultGameplayAbility.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "KnockbackGameplayAbility.generated.h"

/**
 * 넉백 페이로드. 서버가 런치 속도(Force)를 담아 ServerOnly 어빌리티로 LaunchCharacter를 적용한다.
 * (서버 로컬에서만 생성·소비되므로 네트워크 직렬화는 불필요.)
 */
USTRUCT(BlueprintType)
struct FGameplayAbilityTargetData_Knockback : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	// 넉백 런치 속도(cm/s). Dir*Strength + Up*UpwardStrength.
	UPROPERTY()
	FVector Force = FVector::ZeroVector;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayAbilityTargetData_Knockback::StaticStruct();
	}
};

/**
 * 넉백 어빌리티 (플레이어 전용, ServerOnly).
 *  - 서버: hit 판정 → 런치 속도(Force) 계산 → 피격자 ASC에 GameplayEvent(payload) → 서버에서 LaunchCharacter
 *  - 클라는 서버 CMC 보정으로 결과를 받는다(중복 적용 없음).
 * 트리거 태그(AbilityTriggers)는 BP 서브클래스 또는 어빌리티 부여 시점에 GameplayEvent 소스로 설정한다.
 */
UCLASS()
class MAGICULT_API UKnockbackGameplayAbility : public UMagicultGameplayAbility
{
	GENERATED_BODY()

public:
	UKnockbackGameplayAbility();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	// TriggerEventData->TargetData에서 넉백 페이로드를 추출. 없으면 nullptr.
	static const FGameplayAbilityTargetData_Knockback* ExtractPayload(const FGameplayEventData* TriggerEventData);
};
