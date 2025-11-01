// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MagicultCharacterBase.generated.h"

UCLASS()
class MAGICULT_API AMagicultCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AMagicultCharacterBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
