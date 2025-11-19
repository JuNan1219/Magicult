// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "MagicultPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UMagicultInputConfig;

/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMagicultPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

	void CursorDirection();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> MagicultContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	void Move(const FInputActionValue& InputActionValue);

	void CursorTrace();

	FHitResult CursorHit;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UMagicultInputConfig> InputConfig;
};
