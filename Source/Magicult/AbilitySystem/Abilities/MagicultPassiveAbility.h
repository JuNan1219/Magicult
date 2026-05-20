// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultGameplayAbility.h"
#include "MagicultPassiveAbility.generated.h"

/**
 * 패시브 어빌리티 기반 클래스.
 * InstancedPerActor + 명시적 EndAbility 호출 전까지 무한 지속.
 * BP에서 ActivateAbility / EndAbility 이벤트를 오버라이드해 태그 부여·제거 구현.
 */
UCLASS()
class MAGICULT_API UMagicultPassiveAbility : public UMagicultGameplayAbility
{
	GENERATED_BODY()

public:
	UMagicultPassiveAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
