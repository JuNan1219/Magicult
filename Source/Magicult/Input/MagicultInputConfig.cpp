// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultInputConfig.h"

#include "InputAction.h"

const UInputAction* UMagicultInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const auto& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
			return Action.InputAction;
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InpnutTag [%s], on InputConfig [%s]"), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
