// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicCircleInfo.h"

FCircleInfo UMagicCircleInfo::FindMagicCircleInfoForTag(const FGameplayTag& MagicElementTag, bool bLogNotFound) const
{
	for (const FCircleInfo& Info : MagicCircleInformation)
	{
		if (Info.MagicElementTag == MagicElementTag)
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find Info for MagicElementTag [%s]"), *MagicElementTag.ToString());
	}
	
	return FCircleInfo();
}
