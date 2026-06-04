// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "MagicComponent.generated.h"

class UMagicCircleInfo;
class UAttributeSet;
class UNiagaraComponent;

USTRUCT(BlueprintType)
struct FSpellTagLevelPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;
};

USTRUCT(BlueprintType)
struct FSpellState
{
	GENERATED_BODY()

	// 단일 선택 유형 태그 (FGameplayTag)
	// (예: 속성, 방향 등 상호 배타적인 하나의 선택)

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Magic Config")
	FGameplayTag DirectionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Config")
	FGameplayTag SpeedTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Config")
	FSpellTagLevelPair ElementTagPair; // 예: Spell.Element.Fire

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Config")
	FSpellTagLevelPair AmmoTypeTagPair;

	// 다중 선택 효과 태그 (FGameplayTagContainer)
	// (예: 중첩될 수 있는 부가적인 효과나 옵션)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Config")
	TMap<FGameplayTag, int32> EffectTags;

	void Clear()
	{
		DirectionTag = FGameplayTag();
		SpeedTag = FGameplayTag();
		ElementTagPair = FSpellTagLevelPair();
		AmmoTypeTagPair = FSpellTagLevelPair();

		EffectTags.Empty();
	}

	bool HasEffectTag(FGameplayTag InTag) const
	{
		return EffectTags.Contains(InTag);
	}
};
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangedCurrentSpellState, FSpellState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGICULT_API UMagicComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UMagicComponent();

	void AttachMagicCircle();

	UPROPERTY(EditAnywhere, Category = "Magic")
	TObjectPtr<UStaticMeshComponent> MagicCircle;

	FOnChangedCurrentSpellState OnChangedCurrentSpellState;
	void HandleSpellStateChange(FSpellState Current);

	FSpellState CurrentSpellState;

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void SetSpellTag(const FGameplayTag& AbilityTag, int32 Level = 1);

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void SetSpellState(const FSpellState& NewSpellState);

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void SetSpellDirectionTag(const FGameplayTag& DirectionTag);

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void SetSpellSpeedTag(const FGameplayTag& SpeedTag);

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void SetSpellElementTag(const FGameplayTag& ElementTag, int32 Level);

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void SetSpellAmmoTypeTag(const FGameplayTag& AmmoTypeTag, int32 Level);
	UFUNCTION(BlueprintCallable, Category = "Magic")
	void AddSpellEffectTags(const FGameplayTag& EffectTag, int32 Level);
	UFUNCTION(BlueprintCallable, Category = "Magic")
	void ActivateMagicCircle();
	UFUNCTION(BlueprintCallable, Category = "Magic")
	void DeactivateMagicCircle();
	UFUNCTION(BlueprintCallable, Category = "Magic")
	void SetMagicCircleMaterialByTag(const FGameplayTag& ElementTag);
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere, Category = "Magic")
	TObjectPtr<UMagicCircleInfo> MagicCircleMaterials;
};
