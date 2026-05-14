// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionExecutionConfig.h"

bool UActionExecutionConfig::FindActionConfig(ESpellActionType ActionType, FActionConfig& OutActionConfig) const
{
	for (const auto& ActionConfig : ActionConfigs)
	{
		if (ActionConfig.ActionType == ActionType)
		{
			OutActionConfig = ActionConfig;
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("맞는 ActionConfigs가 없습니다. %s"), *UEnum::GetValueAsString(ActionType));
	return false;
}
