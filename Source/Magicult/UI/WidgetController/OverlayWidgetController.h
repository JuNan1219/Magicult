// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultWidgetController.h"
#include "OverlayWidgetController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloatChangedSignature, float, NewFloat);

struct FOnAttributeChangeData;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class MAGICULT_API UOverlayWidgetController : public UMagicultWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependecies() override;

	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnFloatChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnFloatChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnFloatChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnFloatChangedSignature OnMaxManaChanged;
protected:
	void HealthChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;
	
	void ManaChanged(const FOnAttributeChangeData& Data) const;
	void MaxManaChanged(const FOnAttributeChangeData& Data) const;
};
