// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatRuntimeObject.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatRuntimeObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MAGICULT_API ICombatRuntimeObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void PrepareCleanup();
};
