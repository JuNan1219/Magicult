// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MagicInfo.generated.h"


class UGameplayAbility;
class UUGameplayEffect;

USTRUCT(BlueprintType)
struct FMagicultMagicInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();

	/** 마법 사용 가능 여부 **/
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatusTag = FGameplayTag();

	/** 마법 발동 타입(Active, Passive) **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityType = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UTexture2D> Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString AbilityDisplayName = FString();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 LevelRequirement = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};
USTRUCT(BlueprintType)
struct FMagicultSimpleMagicInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();

	/** 마법 사용 가능 여부 **/
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatusTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName = FText();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText ToolTip = FText();

	/**
	 * 모드 목록(Character.State.MagicMode) 페이지 전용 표시값 — 이 모드 카테고리에
	 * 이미 값이 설정돼 있는지. 상세 페이지 엔트리에서는 항상 false로 둔다(의미 없음).
	 * 에디터에서 편집할 값이 아니라 런타임에 계산되므로 EditDefaultsOnly를 안 붙인다.
	 */
	UPROPERTY(BlueprintReadOnly)
	bool bConfigured = false;
};

USTRUCT(BlueprintType)
struct FMagicInfoArray
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FMagicultSimpleMagicInfo> Magics;
};
/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicInfo : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FMagicultMagicInfo> MagicInformation;

	UFUNCTION(BlueprintCallable)
	FMagicultMagicInfo FindMagicInfoForAbilityTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false) const;

	UFUNCTION(BlueprintCallable)
	TArray<FMagicultMagicInfo> FindMagicInfoForInputTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FMagicInfoArray> ModeToMagics;
};
