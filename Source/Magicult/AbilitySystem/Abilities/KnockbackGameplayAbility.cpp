// Fill out your copyright notice in the Description page of Project Settings.

#include "KnockbackGameplayAbility.h"
#include "GameFramework/Character.h"

UKnockbackGameplayAbility::UKnockbackGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// ServerOnly: 서버에서만 넉백을 적용하고, 클라는 CMC 보정으로 결과를 받는다.
	// (클라도 실행하면 서버 보정 속도와 중복돼 오버런이 나므로 서버 단독 적용.)
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	// 넉백 중 새 피격이 오면 덮어쓰도록 재트리거 허용.
	bRetriggerInstancedAbility = true;
}

void UKnockbackGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FGameplayAbilityTargetData_Knockback* Payload = ExtractPayload(TriggerEventData);
	if (!Payload)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector& Force = Payload->Force;

	// ServerOnly라 서버에서만 실행. 일반 CMC 속도를 부여하면 클라는 보정으로 받아 반영한다.
	Character->LaunchCharacter(Force, false, false);

	// 속도만 한 번 부여하고 종료(이후 이동은 CMC가 굴림).
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

const FGameplayAbilityTargetData_Knockback* UKnockbackGameplayAbility::ExtractPayload(
	const FGameplayEventData* TriggerEventData)
{
	if (!TriggerEventData)
	{
		return nullptr;
	}

	for (int32 Index = 0; Index < TriggerEventData->TargetData.Num(); ++Index)
	{
		const FGameplayAbilityTargetData* Data = TriggerEventData->TargetData.Get(Index);
		if (Data && Data->GetScriptStruct() == FGameplayAbilityTargetData_Knockback::StaticStruct())
		{
			return static_cast<const FGameplayAbilityTargetData_Knockback*>(Data);
		}
	}

	return nullptr;
}
