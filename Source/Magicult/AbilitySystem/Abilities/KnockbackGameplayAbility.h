// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultGameplayAbility.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "KnockbackGameplayAbility.generated.h"

/**
 * 넉백 페이로드 (LaunchCharacter 버전).
 * 서버가 넉백 런치 속도(Force)를 담아 보내고, ServerInitiated 활성화로 오너 클라까지 복제된다.
 * 서버·클라 모두 LaunchCharacter(Force)로 동일 속도를 부여한다.
 *  - root motion이 아니라 일반 CMC 속도 → 표준 이동 예측/보정 경로(달리기·점프와 동일)를 탄다.
 *  - root motion 재수렴 경로(오버슛 터지던 곳)를 안 거치므로 보정이 다르게(더 부드러울 수) 동작.
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

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// FVector_NetQuantize(양자화)는 NetCore 모듈 의존이 필요해 풀 정밀도로 직렬화한다.
		Ar << Force;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityTargetData_Knockback>
	: public TStructOpsTypeTraitsBase2<FGameplayAbilityTargetData_Knockback>
{
	enum
	{
		WithNetSerializer = true
	};
};

/**
 * 넉백 어빌리티 (플레이어 전용, LaunchCharacter 버전).
 *
 * 흐름:
 *  - 서버: hit 판정 → 런치 속도(Force) 계산 → 피격자 ASC에 GameplayEvent(payload) 전송
 *  - ServerInitiated → 활성화 + 이벤트 데이터가 오너 클라로 복제
 *  - 서버/클라 양쪽이 LaunchCharacter(Force) → 일반 CMC 속도로 감속(표준 이동 예측)
 *
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
