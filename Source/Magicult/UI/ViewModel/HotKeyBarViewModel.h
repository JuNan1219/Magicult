// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Magicult/Data/MagicInfo.h"
#include "Magicult/Data/SpellTypes.h"
#include "HotKeyBarViewModel.generated.h"

class UMagicInfo;
class UMagicultAbilitySystemComponent;

/**
 * 핫키바가 그릴 파생 상태. "지금 보여야 할 것"(bVisible/PageTag/Entries)과
 * "방금 확정된 것"(CommittedEntry/CommittedSlot, 고스트 연출용. 없으면 INDEX_NONE)을 함께 담는다.
 */
USTRUCT(BlueprintType)
struct FHotKeyBarViewState
{
	GENERATED_BODY()

	/** 핫키바 표시 여부 (Character.State.MagicMode 보유 여부) */
	UPROPERTY(BlueprintReadOnly, Category = "HotKeyBar")
	bool bVisible = false;

	/** 현재 페이지. Character.State.MagicMode = 모드 목록 / Character.Mode.* = 해당 모드의 상세 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "HotKeyBar")
	FGameplayTag PageTag;

	/** 이 페이지가 그려야 할 칸들. 배열 순서 = InputTag.Detail.1~5 */
	UPROPERTY(BlueprintReadOnly, Category = "HotKeyBar")
	TArray<FMagicultSimpleMagicInfo> Entries;

	/**
	 * 이번 변경으로 확정된 항목(고스트 연출 대상).
	 * 확정 당시 그 항목이 있던 "이전 페이지"의 데이터다 — 도착 페이지의 같은 슬롯이 아니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "HotKeyBar")
	FMagicultSimpleMagicInfo CommittedEntry;

	/** 확정 항목이 있던 슬롯(0~4). 확정이 없으면 INDEX_NONE. */
	UPROPERTY(BlueprintReadOnly, Category = "HotKeyBar")
	int32 CommittedSlot = INDEX_NONE;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotKeyBarStateChangedSignature, const FHotKeyBarViewState&, NewState);

/**
 * 마법 모드 핫키바 ViewModel. GAS에서 흩어져 오는 신호(MagicMode/Character.Mode.SpellState 확정)를
 * 하나의 파생 상태로 통합해 View에 단일 델리게이트로 전달한다.
 * 신호 구독은 UOverlayWidgetController가 하고, 이 클래스는 그 어댑터 호출만 받는다.
 */
UCLASS(BlueprintType)
class MAGICULT_API UHotKeyBarViewModel : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 필터를 통과한 신호마다 항상 발행한다(diff 비교 없음) — 이 안전성은 게임 규칙(Character.Mode
	 * 자기 블록)에 암묵적으로 의존한다. 규칙이 풀려도 죽은 버튼이 생기지 않도록 소스 필터만으로 중복을 거른다.
	 */
	UPROPERTY(BlueprintAssignable, Category = "HotKeyBar")
	FOnHotKeyBarStateChangedSignature OnStateChanged;

	/** UFUNCTION은 참조 반환을 지원하지 않아 값으로 돌려준다(칸 5개짜리라 복사 비용은 무시 가능). */
	UFUNCTION(BlueprintPure, Category = "HotKeyBar")
	FHotKeyBarViewState GetState() const { return State; }

	void Initialize(UMagicInfo* InMagicInfo, UMagicultAbilitySystemComponent* InAbilitySystemComponent);

	/** 현재 상태를 그대로 한 번 발행한다. View가 늦게 붙었을 때 초기 동기화용. */
	void BroadcastCurrent();

	// ─ 입력 어댑터: UOverlayWidgetController가 GAS 이벤트를 받아 호출한다 ─

	/** Character.State.MagicMode 보유 변화 */
	void HandleMagicModeChanged(bool bActive);

	/** Character.Mode.* 태그 변화. NewCount는 RegisterGenericGameplayTagEvent가 주는 태그 카운트 그대로. */
	void HandleModeTagChanged(const FGameplayTag& ModeTag, int32 NewCount);

	/** MagicComponent::OnChangedCurrentSpellState 의 (NewSpellState, ChangedTag) */
	void HandleSpellTagCommitted(const FSpellState& NewSpellState, const FGameplayTag& CommittedTag);

private:
	/**
	 * ModeToMagics에서 PageTag의 칸 목록을 만들고 StatusTag를 채운다. 키가 없으면 false.
	 * PageTag가 모드 목록이면 각 칸의 bConfigured도 CachedSpellState 기준으로 채운다.
	 */
	bool BuildEntries(const FGameplayTag& PageTag, TArray<FMagicultSimpleMagicInfo>& OutEntries);

	/** AbilityTag가 일치하는 칸의 인덱스. 없으면 INDEX_NONE. */
	static int32 FindSlot(const TArray<FMagicultSimpleMagicInfo>& InEntries, const FGameplayTag& AbilityTag);

	/**
	 * ModeTag(Character.Mode.*)가 가리키는 카테고리가 CachedSpellState에 이미 설정돼 있는지.
	 * Effect는 다중 선택이라 "하나라도 있으면" 설정된 것으로 본다.
	 */
	bool IsCategoryConfigured(const FGameplayTag& ModeTag) const;

	/**
	 * 상태 갱신 + 발행의 단일 진입점.
	 * 후속으로 로컬 예측을 도입할 때 예측 경로와 서버 확정 경로가 여기서 합류해야 재조정이 가능하다.
	 */
	void ApplyAndBroadcast(bool bNewVisible, const FGameplayTag& NewPageTag,
		int32 InCommittedSlot = INDEX_NONE,
		const FMagicultSimpleMagicInfo& InCommitted = FMagicultSimpleMagicInfo());

	UPROPERTY()
	TObjectPtr<UMagicInfo> MagicInfo;

	UPROPERTY()
	TObjectPtr<UMagicultAbilitySystemComponent> AbilitySystemComponent;

	FHotKeyBarViewState State;

	/**
	 * MagicComponent::CurrentSpellState의 최신 스냅샷. 소유 클라는 COND_SkipOwner 복제라
	 * LocalPredicted 어빌리티가 이미 로컬로 채워준 값이라서, 이 캐시는 RTT 대기 없이 갱신된다.
	 * 모드 목록 페이지의 "이미 설정됨" 표시(bConfigured)에만 쓴다.
	 */
	FSpellState CachedSpellState;
};
