// Fill out your copyright notice in the Description page of Project Settings.

#include "KnockbackGameplayAbility.h"
#include "GameFramework/Character.h"

UKnockbackGameplayAbility::UKnockbackGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// 넉백은 당하는 동작이라 클라가 예측할 수 없다. 서버가 활성화하고 클라가 따라간다.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	// 넉백 진행 중 새 피격이 오면 덮어쓸 수 있도록 재트리거 허용.
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

	// 서버 LaunchCharacter의 이동(속도) 복제가 GameplayEvent보다 먼저 클라에 도착하는 경우가 있다.
	// 이때 클라가 또 LaunchCharacter를 적용하면 속도가 중복으로 더해진다.
	// 현재 속도를 목표 속도(Force)에 내적해 그 값이 |Force|^2의 80% 이상이면
	// 이미 충분히 반영된 것으로 보고 클라 측 LaunchCharacter를 건너뛴다.
	// 중력으로 계속 변하는 Z(수직) 속도는 비교에서 제외하고 수평(X, Y)만 본다.
	if (!Character->HasAuthority())
	{
		const FVector2D CurrentVelocity2D(Character->GetVelocity());
		const FVector2D Force2D(Force);
		if ((CurrentVelocity2D | Force2D) >= 0.8f * Force2D.SizeSquared())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
	}

	// 일반 CMC 속도 부여. 이후 마찰/감속은 표준 이동 예측 경로가 처리한다.
	Character->LaunchCharacter(Force, true, true);

	// 한 방 속도만 부여하면 끝. 어빌리티는 즉시 종료(이후 이동은 CMC가 굴림).
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
