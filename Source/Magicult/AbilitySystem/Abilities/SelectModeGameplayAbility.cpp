// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectModeGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Magicult/Data/InputToTagConfig.h"
#include "Magicult/Data/MagicInfo.h"
#include "Magicult/Player/MagicultPlayerController.h"


void USelectModeGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!InputToTag)
	{
		UE_LOG(LogTemp, Warning, TEXT("You need to set InputToTag %s"), *ActorInfo->AvatarActor->GetName());
		return;
	}
	if (TriggerEventData && InputToTag)
	{
		bool bFound = false;
		const FInputToSelect InputToSelect = InputToTag->FindTagToInput(TriggerEventData->EventTag, bFound);

        if (bFound && InputToSelect.EffectClass)
        {
            const FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
            const FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(InputToSelect.EffectClass, 1, ContextHandle);
            AppliedEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
        }
	}
}

void USelectModeGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AppliedEffectHandle.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(AppliedEffectHandle);
	}
}
