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

	// causer의 위치(수집/폭발 중심). Actor는 자기 위치, 논리 causer(AttackInstance)는 주입된 origin.
	// 정책 해석(DamageCauser)이 UObject causer의 위치를 찾는 데 쓰인다.
	UFUNCTION(BlueprintCallable, Category = "Attack")
	virtual FVector GetAttackOrigin() const;
};
