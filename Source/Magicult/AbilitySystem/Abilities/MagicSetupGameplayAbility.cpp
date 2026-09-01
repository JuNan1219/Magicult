// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicSetupGameplayAbility.h"
#include "Magicult/Interface/MagicInterface.h"
#include "Magicult/Component/MagicComponent.h"
#include "Magicult/MagicultGameplayTags.h"

UMagicSetupGameplayAbility::UMagicSetupGameplayAbility()
{
	// NetExecutionPolicy는 LocalPredicted: 소유 클라에서도 ActivateAbility 본문이 실행되어
	// SetSpellTag가 서버와 소유 클라 양쪽에서 각각 도니, UI가 네트워크 왕복 없이 즉시 반응한다.
	// CurrentSpellState는 Replicated라 서버 값으로 최종 동기화된다.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationBlockedTags.AddTag(FMagicultGameplayTags::Get().Debuff_Stunned);
	ActivationBlockedTags.AddTag(FMagicultGameplayTags::Get().Character_State_Dead);
}

void UMagicSetupGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// bIsValid는 서버(HasAuthority)와 소유 클라(IsLocallyControlled) 양쪽을 통과시킨다 — 의도된 로컬 예측이다.
	// 서버 값이 Replicated CurrentSpellState로 최종 동기화되므로 클라 실행은 UI 반응성을 위한 낙관적 갱신일 뿐이다.
	bool bIsValid = (HasAuthority(&ActivationInfo) || ActorInfo->IsLocallyControlled()) && ActorInfo != nullptr && IsValid(ActorInfo->AvatarActor.Get()) && ActorInfo->AvatarActor.Get()->Implements<UMagicInterface>();
	if (bIsValid)
	{
		UMagicComponent* MagicComponent = IMagicInterface::Execute_GetMagicComponent(ActorInfo->AvatarActor.Get());
		for (const auto& AbilityTag : AbilityTags)
		{
			MagicComponent->SetSpellTag(AbilityTag, GetAbilityLevel());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
