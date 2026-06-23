// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicSetupGameplayAbility.h"
#include "Magicult/Interface/MagicInterface.h"
#include "Magicult/Component/MagicComponent.h"

UMagicSetupGameplayAbility::UMagicSetupGameplayAbility()
{
	// NetExecutionPolicy는 "입력→서버 라우팅"을 위해서만 LocalPredicted로 둔다.
	// (GameplayEvent 트리거는 로컬에서만 처리되므로, 클라 입력이 서버에 닿으려면 LocalPredicted가 필요하다.)
	// 실제 SpellTag 적용은 ActivateAbility에서 HasAuthority로 가드되어 서버에서만 수행된다(무예측).
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UMagicSetupGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// SpellTag 적용은 서버 권위 컴포넌트(MagicComponent) 직접 호출이므로 서버에서만 수행한다.
	// (LocalPredicted라 클라 카피에서도 본문이 돌지만, 가드로 클라 중복 적용을 막는다.)
	bool bIsValid = HasAuthority(&ActivationInfo) && ActorInfo != nullptr && IsValid(ActorInfo->AvatarActor.Get()) && ActorInfo->AvatarActor.Get()->Implements<UMagicInterface>();
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
