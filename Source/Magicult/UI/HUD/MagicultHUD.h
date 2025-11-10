// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MagicultHUD.generated.h"

class UMagicultUserWidget;
/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;


	UPROPERTY()
	TObjectPtr<UMagicultUserWidget> OverlayWidget;


private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UMagicultUserWidget> OverlayWidgetClass;
};
