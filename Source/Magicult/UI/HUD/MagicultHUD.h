// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MagicultHUD.generated.h"

class UMagicultUserWidget;
class UOverlayWidgetController;
class UMagicMenuWidgetController;
class UAbilitySystemComponent;
class UAttributeSet;
struct FWidgetControllerParams;
/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultHUD : public AHUD
{
	GENERATED_BODY()

public:
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	UMagicMenuWidgetController* GetMagicMenuWidgetController(const FWidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

	UFUNCTION(BlueprintImplementableEvent)
	void HandleMagicMenu();
private:
	UPROPERTY()
	TObjectPtr<UMagicultUserWidget> OverlayWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UMagicultUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY()
	TObjectPtr<UMagicMenuWidgetController> MagicMenuWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UMagicMenuWidgetController> MagicMenuWidgetControllerClass;
};
