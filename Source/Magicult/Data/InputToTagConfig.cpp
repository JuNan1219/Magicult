// Fill out your copyright notice in the Description page of Project Settings.


#include "InputToTagConfig.h"

const FInputToSelect UInputToTagConfig::FindTagToInput(const FGameplayTag& InputTag, bool& bFound) const
{
	for (const auto& Info : InputToSelects)
	{
		if (Info.InputTag == InputTag)
		{
			bFound = true;
			return Info;
		}
	}

	bFound = false;
	return FInputToSelect();
}
