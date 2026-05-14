// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicSetupGameplayAbility.h"
#include "Magicult/Interface/MagicInterface.h"
#include "Magicult/Component/MagicComponent.h"

void UMagicSetupGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	bool bIsValid = ActorInfo != nullptr && IsValid(ActorInfo->AvatarActor.Get()) && ActorInfo->AvatarActor.Get()->Implements<UMagicInterface>();
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
