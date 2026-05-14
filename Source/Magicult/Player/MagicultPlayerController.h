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
class UMagicultAbilitySystemComponent;

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

	UFUNCTION(BlueprintCallable)
	void AddMagicDetailInputContext();
	UFUNCTION(BlueprintCallable)
	void RemoveMagicDetailInputContext();

	UPROPERTY(EditAnywhere)
	float RotationSpeed = 90.f;
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> MagicultContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> MagicDetailContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> RotationAction;

	void Move(const FInputActionValue& InputActionValue);
	void Rotation(const FInputActionValue& InputActionValue);

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MenuAction;

	void HandleMenuInput(const FInputActionValue& InputActionValue);

	void CursorTrace();

	FHitResult CursorHit;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UMagicultInputConfig> InputConfig;

	UPROPERTY()
	TWeakObjectPtr<UMagicultAbilitySystemComponent> MagicultAbilitySystemComponent;

	UMagicultAbilitySystemComponent* GetASC();

	UFUNCTION(Server, Unreliable)
	void ServerSetFacingYaw(float Yaw);

	float LastSentYaw = 0.f;
};
