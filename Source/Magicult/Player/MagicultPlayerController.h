// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MagicultPlayerController.generated.h"

class UInputMappingContext;

/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMagicultPlayerController();

protected:
	virtual void BeginPlay() override;


private:
	UPROEPRTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> MagicultContext;
};
