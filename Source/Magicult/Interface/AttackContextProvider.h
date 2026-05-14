// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Magicult/Data/AttackSpec.h"
#include "AttackContextProvider.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UAttackContextProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MAGICULT_API IAttackContextProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, Category = "Attack")
	virtual FAttackSpec GetAttackSpec() const;

	UFUNCTION(BlueprintCallable, Category = "Attack")
	virtual FVector GetActorVelocity() const;
};
