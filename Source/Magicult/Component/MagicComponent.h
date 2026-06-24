// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "Magicult/Data/SpellTypes.h"
#include "MagicComponent.generated.h"

class UMagicCircleInfo;
class UAttributeSet;
class UNiagaraComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChangedCurrentSpellState, FSpellState, FGameplayTag);

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
	void HandleSpellStateChange(FSpellState Current, FGameplayTag ChangedTag);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 서버에서만 갱신되고(권위), 소유 클라는 로컬 예측으로 이미 같은 값을 들고 있어 COND_SkipOwner로 복제한다.
	// 몬스터처럼 소유 클라가 없는 경우엔 모든 클라에게 복제되어 마법진 비주얼이 보인다.
	UPROPERTY(ReplicatedUsing = OnRep_CurrentSpellState)
	FSpellState CurrentSpellState;

	UFUNCTION()
	void OnRep_CurrentSpellState();

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

	// MagicCircle->SetVisibility는 복제 프로퍼티가 아니라서, 권위 머신(서버)에서만 호출되면
	// 다른 클라(특히 소유 클라가 없는 몬스터)에는 절대 반영되지 않는다. 이 bool로 대신 복제한다.
	UPROPERTY(ReplicatedUsing = OnRep_MagicCircleActive)
	bool bMagicCircleActive = false;

	UFUNCTION()
	void OnRep_MagicCircleActive();

	UFUNCTION(BlueprintCallable, Category = "Magic")
	void ClearSpellState();

protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere, Category = "Magic")
	TObjectPtr<UMagicCircleInfo> MagicCircleMaterials;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	void UpdateMagicCircleSpeed(const FGameplayTag& SpeedTag);
	void UpdateMagicCircleDirection(const FGameplayTag& DirectionTag);
};
