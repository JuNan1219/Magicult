// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "DebuffVisualComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FDebuffVisualEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag DebuffTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> NiagaraSystem;
};

/**
 * 디버프 활성화 시 Niagara 파티클을 카메라 기준 빌보드 방향으로 가운데 정렬 배치하는 컴포넌트.
 * MagicultCharacterBase에 기본 부착. 에디터 기즈모로 높이/위치 조정 가능.
 * + 버튼으로 추가 가능.
 */
UCLASS(ClassGroup=(Magicult), meta=(BlueprintSpawnableComponent))
class MAGICULT_API UDebuffVisualComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDebuffVisualComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void BeginPlay() override;

protected:
	/** 감지할 디버프 태그와 대응하는 Niagara 에셋 쌍 */
	UPROPERTY(EditDefaultsOnly, Category="Debuff Visual")
	TArray<FDebuffVisualEntry> DebuffVisuals;

	/** 아이콘 간격 (cm) */
	UPROPERTY(EditDefaultsOnly, Category="Debuff Visual")
	float Spacing = 80.f;

	/** 디버프 추가 시 Blueprint 확장 포인트 */
	UFUNCTION(BlueprintNativeEvent, Category="Debuff Visual")
	void OnDebuffAdded(FGameplayTag DebuffTag);
	virtual void OnDebuffAdded_Implementation(FGameplayTag DebuffTag) {}

	/** 디버프 제거 시 Blueprint 확장 포인트 */
	UFUNCTION(BlueprintNativeEvent, Category="Debuff Visual")
	void OnDebuffRemoved(FGameplayTag DebuffTag);
	virtual void OnDebuffRemoved_Implementation(FGameplayTag DebuffTag) {}

private:
	void TryInitializeWithASC();
	void OnDebuffTagChanged(const FGameplayTag Tag, int32 NewCount);
	void AddDebuffVisual(const FGameplayTag& Tag);
	void RemoveDebuffVisual(const FGameplayTag& Tag);

	/** 활성 디버프 Niagara 컴포넌트들을 빌보드 방향으로 가운데 정렬 */
	void RefreshLayout();

	/** 활성 디버프 태그 → Niagara 컴포넌트 (GC 보호를 위해 UPROPERTY) */
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UNiagaraComponent>> ActiveDebuffComponents;

	/** 정렬 순서 유지용 배열 (추가된 순서 기반) */
	TArray<FGameplayTag> ActiveDebuffOrder;

	/** EndPlay 시 이벤트 해제를 위한 핸들 보관 */
	TMap<FGameplayTag, FDelegateHandle> TagEventHandles;

	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};
