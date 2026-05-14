// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MagicultInputConfig.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActionInputBindingFlags : uint8
{
	// 아무것도 바인딩하지 않음
	None = 0,
	// 버튼을 눌렀을 때 (Started/Pressed)
	BindPress = 1 << 0,  // Binary: 0001
	// 버튼을 누르고 있을 때 (Triggered/Held)
	BindHeld = 1 << 1,  // Binary: 0010
	// 버튼에서 손을 뗐을 때 (Completed/Released)
	BindRelease = 1 << 2,  // Binary: 0100
};

// 열거형이 비트마스크로 인식되도록 별도의 헬퍼 정의 (GAS 프로젝트에서 필요)
ENUM_CLASS_FLAGS(EActionInputBindingFlags)

USTRUCT(BlueprintType)
struct FMagicultInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();

	/** UPROPERTY 속성에 Bitmask와 BitmaskEnum을 지정하여 에디터에서 드롭다운 체크박스 형태로 표시되도록 합니다.**/
	UPROPERTY(EditDefaultsOnly, meta = (Bitmask, BitmaskEnum = "/Script/Magicult.EActionInputBindingFlags"))
	uint8 AllowedBindings = (uint8)EActionInputBindingFlags::BindPress;
};

/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicultInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FMagicultInputAction> AbilityInputActions;
};
