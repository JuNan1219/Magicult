// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrapButton.generated.h"

class ATrapActor;

UCLASS()
class MAGICULT_API ATrapButton : public AActor
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void PressButton();

    virtual void PressButton_Implementation();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TObjectPtr<ATrapActor>> LinkedTraps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOneShot = true;

    bool bPressed = false;

};
