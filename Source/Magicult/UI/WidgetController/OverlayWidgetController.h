// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultWidgetController.h"
#include "Magicult/Data/MagicInfo.h"
#include "Magicult/Data/SpellTypes.h"
#include "OverlayWidgetController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloatChangedSignature, float, NewFloat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTagChangedSignature, FGameplayTag, Tag, FMagicInfoArray, MagicInfoArray, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpellStateChangedSignature, FSpellState, NewSpellState, FGameplayTag, ChangedTag);

struct FOnAttributeChangeData;
class UMagicComponent;
class UHotKeyBarViewModel;

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

	// MagicComponent::OnChangedCurrentSpellState를 중계 — 마법 설정의 마지막 단계(태그 슬롯이 없는 단계)를 View에 알린다.
	UPROPERTY(BlueprintAssignable, category = "GAS")
	FOnSpellStateChangedSignature OnSpellStateChanged;

	// 마법 모드 핫키바 전용 ViewModel. 흩어진 GAS 신호를 단일 파생 상태로 통합한다.
	// View(WBP_HotKey)는 ASC/태그를 직접 보지 않고 이 객체의 OnStateChanged만 구독한다.
	UPROPERTY(BlueprintReadOnly, Category = "HotKeyBar")
	TObjectPtr<UHotKeyBarViewModel> HotKeyBarViewModel;

protected:

private:
	// 클라이언트에서 BindCallbacksToDependencies 시점에 Pawn이 아직 복제 안 됐을 때,
	// Pawn이 잡히는 시점을 기다리기 위한 OnPossessedPawnChanged 콜백.
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	// Pawn의 MagicComponent::OnChangedCurrentSpellState를 구독해 OnSpellStateChanged로 중계한다.
	// 즉시 호출/델리게이트/재포제션으로 중복 호출돼도 BoundMagicComponent 가드로 한 번만 등록된다.
	void BindToMagicComponent(APawn* OwnerPawn);

	// 같은 MagicComponent에 람다가 중복 등록되는 걸 막는 가드.
	TWeakObjectPtr<UMagicComponent> BoundMagicComponent;
};
