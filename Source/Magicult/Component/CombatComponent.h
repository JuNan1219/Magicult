// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Magicult/MagicultAbilityTypes.h"
#include "CombatComponent.generated.h"

struct FCombatResolveResult;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGICULT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

protected:
	virtual void BeginPlay() override;

public:	

		
};
