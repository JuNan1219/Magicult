// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Magicult/Data/ActionExecutionConfig.h"
#include "MagicultGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicultGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	TObjectPtr<UActionExecutionConfig> ActionExecutionConfig;

};
