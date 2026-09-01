// Fill out your copyright notice in the Description page of Project Settings.


#include "HotKeyBarViewModel.h"

#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/MagicultGameplayTags.h"

void UHotKeyBarViewModel::Initialize(UMagicInfo* InMagicInfo, UMagicultAbilitySystemComponent* InAbilitySystemComponent)
{
	MagicInfo = InMagicInfo;
	AbilitySystemComponent = InAbilitySystemComponent;
}

void UHotKeyBarViewModel::BroadcastCurrent()
{
	OnStateChanged.Broadcast(State);
}

void UHotKeyBarViewModel::HandleMagicModeChanged(bool bActive)
{
	if (bActive)
	{
		// 마법 모드 진입 → 모드 목록 페이지. 진입 자체는 "확정"이 아니라 고스트 연출이 없다.
		ApplyAndBroadcast(true, FMagicultGameplayTags::Get().Character_State_MagicMode);
		return;
	}

	// 종료 → 숨김. 페이지/칸을 비워 다음 진입 때 이전 상태가 새지 않게 한다.
	ApplyAndBroadcast(false, FGameplayTag());
}

void UHotKeyBarViewModel::HandleModeTagChanged(const FGameplayTag& ModeTag, int32 NewCount)
{
	// Character.Mode.* 제거는 직접 모드 전환과 디테일 확정(CancelAbilitiesWithTag로 인한 자동 캔슬)
	// 양쪽에서 발생하며, 둘 다 무조건 무시해도 된다 — 페이지 복귀는 HandleSpellTagCommitted /
	// HandleMagicModeChanged(false)가 담당한다. NewCount>0이 아니라 ==1(0→1 전이)만 커밋으로 보는
	// 이유: 서버 확정 GE 복제 시 GAS 재조정이 "지우고 다시 추가"하면 카운트가 1→0→1로 움직인다.
	if (NewCount != 1)
	{
		return;
	}

	if (!State.bVisible)
	{
		return;
	}

	// 고스트 대상은 "유저가 실제로 누른 아이콘" = 모드 목록에서 이 모드에 해당하는 칸이다.
	// 도착 페이지의 같은 슬롯을 쓰면 안 된다. 모드 목록의 1번은 "방향 모드"지만
	// Direction 페이지의 1번은 Magic.Direction.0 이라, 다른 항목이 선택된 것처럼 보인다.
	TArray<FMagicultSimpleMagicInfo> ModeListEntries;
	BuildEntries(FMagicultGameplayTags::Get().Character_State_MagicMode, ModeListEntries);

	const int32 Slot = FindSlot(ModeListEntries, ModeTag);
	if (Slot == INDEX_NONE)
	{
		// 연출 대상만 못 찾은 것이므로 상태 전환은 그대로 진행한다.
		UE_LOG(LogTemp, Warning,
			TEXT("[HotKeyBar] 모드 목록에서 %s 에 해당하는 칸을 찾지 못했다. "
				"DA_MagicInfo의 모드 목록 엔트리 AbilityTag를 확인할 것. 고스트 연출만 생략한다."),
			*ModeTag.ToString());

		ApplyAndBroadcast(true, ModeTag);
		return;
	}

	ApplyAndBroadcast(true, ModeTag, Slot, ModeListEntries[Slot]);
}

void UHotKeyBarViewModel::HandleSpellTagCommitted(const FSpellState& NewSpellState, const FGameplayTag& CommittedTag)
{
	// 캐시는 조건 없이 갱신한다 — 아래 CommittedTag 분기와 무관하게 항상 최신 상태를 반영해야
	// 모드 목록의 bConfigured 표시가 정확하다.
	CachedSpellState = NewSpellState;

	// ClearSpellState()는 마법 모드 진입 시 빈 ChangedTag로도 브로드캐스트한다 — 확정이 아니므로
	// 연출/페이지 전환은 안 하지만, 모드 목록을 보고 있다면 bConfigured만 새로고침한다.
	// GA_MagicMode의 "모드 진입 태그 부여"와 "ClearSpellState 호출" 순서를 BP 그래프에 둬 C++에서
	// 보장 못 하므로, 태그가 먼저 와도 뒤이은 이 브로드캐스트가 표시를 바로잡는다.
	if (!CommittedTag.IsValid())
	{
		if (State.bVisible && State.PageTag == FMagicultGameplayTags::Get().Character_State_MagicMode)
		{
			ApplyAndBroadcast(true, State.PageTag);
		}
		return;
	}

	if (!State.bVisible)
	{
		return;
	}

	// 이 시점의 State.Entries는 아직 상세 목록이다. 페이지를 바꾸기 전에 연출 대상을 먼저 집는다.
	const int32 Slot = FindSlot(State.Entries, CommittedTag);
	const FMagicultSimpleMagicInfo Committed = State.Entries.IsValidIndex(Slot)
		? State.Entries[Slot]
		: FMagicultSimpleMagicInfo();

	// 상세 확정 → 모드 목록으로 복귀. 이때 BuildEntries가 방금 갱신된 CachedSpellState로
	// bConfigured를 다시 계산하므로, 방금 고른 항목이 곧바로 "설정됨"으로 표시된다.
	ApplyAndBroadcast(true, FMagicultGameplayTags::Get().Character_State_MagicMode, Slot, Committed);
}

void UHotKeyBarViewModel::ApplyAndBroadcast(bool bNewVisible, const FGameplayTag& NewPageTag,
	int32 InCommittedSlot, const FMagicultSimpleMagicInfo& InCommitted)
{
	State.bVisible = bNewVisible;
	State.PageTag = NewPageTag;
	State.Entries.Reset();

	if (bNewVisible && NewPageTag.IsValid())
	{
		BuildEntries(NewPageTag, State.Entries);
	}

	State.CommittedEntry = InCommitted;
	State.CommittedSlot = InCommittedSlot;

	// diff 없이 항상 발행한다 — 헤더의 OnStateChanged 주석 참고.
	OnStateChanged.Broadcast(State);
}

bool UHotKeyBarViewModel::BuildEntries(const FGameplayTag& PageTag, TArray<FMagicultSimpleMagicInfo>& OutEntries)
{
	OutEntries.Reset();

	if (!MagicInfo)
	{
		return false;
	}

	// 널 체크는 필수다. ModeToMagics에 없는 Character.Mode.* 태그가 붙으면
	// 이전 구현(UOverlayWidgetController)은 여기서 그대로 크래시했다.
	const FMagicInfoArray* Found = MagicInfo->ModeToMagics.Find(PageTag);
	if (!Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HotKeyBar] DA_MagicInfo의 ModeToMagics에 %s 키가 없다."), *PageTag.ToString());
		return false;
	}

	OutEntries = Found->Magics;

	const FMagicultGameplayTags& Tags = FMagicultGameplayTags::Get();

	// 모드 목록은 항상 선택 가능하고, 상세 목록만 어빌리티 보유 여부로 잠금을 판정한다(기존 동작 유지).
	const bool bIsModeList = (PageTag == Tags.Character_State_MagicMode);

	for (FMagicultSimpleMagicInfo& Entry : OutEntries)
	{
		if (bIsModeList)
		{
			Entry.StatusTag = Tags.Magic_Status_Unlocked;
			Entry.bConfigured = IsCategoryConfigured(Entry.AbilityTag);
			continue;
		}

		const FGameplayTag Status = AbilitySystemComponent
			? AbilitySystemComponent->GetStatusFromAbilityTag(Entry.AbilityTag)
			: FGameplayTag();

		Entry.StatusTag = Status.IsValid() ? Tags.Magic_Status_Unlocked : Tags.Magic_Status_Locked;
	}

	return true;
}

bool UHotKeyBarViewModel::IsCategoryConfigured(const FGameplayTag& ModeTag) const
{
	// 모드 목록 엔트리의 AbilityTag가 Character.Mode.* 그대로라는 전제다(FindSlot과 동일 전제).
	// DA_MagicInfo에서 실제로 다르게 잡혀 있으면 여기서는 조용히 false를 반환한다 — 이 값은
	// 표시 힌트일 뿐 상태 전환에 영향을 주지 않아, FindSlot처럼 경고 로그를 매번 찍으면
	// 모드 목록을 볼 때마다(=BuildEntries 호출마다) 스팸이 되므로 넣지 않았다.
	const FMagicultGameplayTags& Tags = FMagicultGameplayTags::Get();

	if (ModeTag == Tags.Character_Mode_Direction)
	{
		return CachedSpellState.DirectionTag.IsValid();
	}
	if (ModeTag == Tags.Character_Mode_Speed)
	{
		return CachedSpellState.SpeedTag.IsValid();
	}
	if (ModeTag == Tags.Character_Mode_Element)
	{
		return CachedSpellState.ElementTagPair.Tag.IsValid();
	}
	if (ModeTag == Tags.Character_Mode_AmmoType)
	{
		return CachedSpellState.AmmoTypeTagPair.Tag.IsValid();
	}
	if (ModeTag == Tags.Character_Mode_Effect)
	{
		// 다중 선택 카테고리 — 하나라도 있으면 설정된 것으로 본다(개수는 표시하지 않는다).
		return CachedSpellState.EffectTags.Num() > 0;
	}

	return false;
}

int32 UHotKeyBarViewModel::FindSlot(const TArray<FMagicultSimpleMagicInfo>& InEntries, const FGameplayTag& AbilityTag)
{
	if (!AbilityTag.IsValid())
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < InEntries.Num(); ++Index)
	{
		if (InEntries[Index].AbilityTag == AbilityTag)
		{
			return Index;
		}
	}

	// 데이터가 상위 태그로 잡혀 있을 수 있어 계층 매칭으로 한 번 더 본다.
	for (int32 Index = 0; Index < InEntries.Num(); ++Index)
	{
		if (InEntries[Index].AbilityTag.IsValid() && AbilityTag.MatchesTag(InEntries[Index].AbilityTag))
		{
			return Index;
		}
	}

	return INDEX_NONE;
}
