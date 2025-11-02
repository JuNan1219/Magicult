// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultCharacterBase.h"
#include "MagicultCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultCharacter : public AMagicultCharacterBase
{
	GENERATED_BODY()
public:
	AMagicultCharacter();

	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USpringArmComponent> CameraArm;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCameraComponent> Camera;
};
