// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultWidgetController.h"
#include "Magicult/Data/MagicInfo.h"
#include "OverlayWidgetController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloatChangedSignature, float, NewFloat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTagChangedSignature, FGameplayTag, Tag, FMagicInfoArray, MagicInfoArray, int32, NewCount);

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
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnFloatChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnFloatChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnFloatChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnFloatChangedSignature OnMaxManaChanged;

	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnTagChangedSignature OnTagChanged;

	// 모드 전환 UI(아이콘) 애니메이션 재생 중에는 View가 이 함수로 입력 차단을 알린다.
	// View는 ASC/태그를 직접 건드리지 않고 항상 Controller를 거친다.
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void SetModeInputBlocked(bool bBlocked);
protected:

};
