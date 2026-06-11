// Fill out your copyright notice in the Description page of Project Settings.

#include "ImmunityPolicy.h"

bool UImmunityPolicy::FindImmunityTag(const FGameplayTag& Tag, FGameplayTag& OutImmunityTag) const
{
	for (const FImmunityRule& Rule : ImmunityRules)
	{
		if (Rule.SubjectTag == Tag)
		{
			OutImmunityTag = Rule.GrantedTag;
			return true;
		}
	}
	return false;
}
