// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicInfo.h"

FMagicultMagicInfo UMagicInfo::FindMagicInfoForAbilityTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const FMagicultMagicInfo& Info : MagicInformation)
	{
		if (Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find info for MagicTag [%s] on MagicInfo [%s]"), *AbilityTag.ToString(), *GetNameSafe(this));
	}

    return FMagicultMagicInfo();
}

TArray<FMagicultMagicInfo> UMagicInfo::FindMagicInfoForInputTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	TArray<FMagicultMagicInfo> Result;
	for (const FMagicultMagicInfo& Info : MagicInformation)
	{
		if (Info.InputTag == InputTag)
		{
			Result.Add(Info);
		}
	}

	if (bLogNotFound && Result.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find info for InputTag [%s] on MagicInfo [%s]"), *InputTag.ToString(), *GetNameSafe(this));
	}

	return Result;
}
