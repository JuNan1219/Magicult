#include "SpellTriggerMatcher.h"

// Context 플래그 비트 정의 (ESpellActionRequireFlags 제거 후 내부 사용)
namespace SpellContextFlags
{
	constexpr uint8 None           = 0;
	constexpr uint8 HasSourceASC   = 1 << 0;
	constexpr uint8 HasTargetASC   = 1 << 1;
	constexpr uint8 HasSourceActor = 1 << 2;
	constexpr uint8 HasTargetActor = 1 << 3;
	constexpr uint8 HasDamageCauser= 1 << 4;
	constexpr uint8 HasHitResult   = 1 << 5;
}

void FSpellTriggerMatcher::CollectActions(const TArray<FSpellTriggerActionBinding>& Bindings, const FSpellTriggerEventData& EventData, TArray<FSpellActionSpec>& OutActions)
{
	for (const FSpellTriggerActionBinding& Binding : Bindings)
	{
		TArray<FSpellActionSpec> ActionSpecs;
		if (!TryCollectActions(Binding, EventData, ActionSpecs))
		{
			continue;
		}

		OutActions.Append(ActionSpecs);
	}
}

bool FSpellTriggerMatcher::TryCollectActions(const FSpellTriggerActionBinding& Binding, const FSpellTriggerEventData& EventData, TArray<FSpellActionSpec>& OutActions)
{
	if (Binding.Trigger.TriggerTag != EventData.TriggerTag)
	{
		return false;
	}

	if (!Binding.Trigger.TriggerContextTags.IsEmpty() &&
		!EventData.TriggerContextTags.HasAll(Binding.Trigger.TriggerContextTags))
	{
		return false;
	}

	const uint8 CurrentFlags = MakeContextFlags(EventData.SpellContext);

	for (const FSpellActionSpec& ActionSpec : Binding.Actions)
	{
		const uint8 RequiredFlags = GetRequiredFlagsForActionType(ActionSpec.ActionType);

		if ((CurrentFlags & RequiredFlags) != RequiredFlags)
		{
			continue;
		}
		OutActions.Add(ActionSpec);
	}

	return !OutActions.IsEmpty();
}

uint8 FSpellTriggerMatcher::MakeContextFlags(const FSpellContext& Context)
{
	uint8 Flags = SpellContextFlags::None;

	if (Context.SourceAbilitySystemComponent.IsValid())
		Flags |= SpellContextFlags::HasSourceASC;

	if (Context.TargetAbilitySystemComponent.IsValid())
		Flags |= SpellContextFlags::HasTargetASC;

	if (Context.SourceActor.IsValid())
		Flags |= SpellContextFlags::HasSourceActor;

	if (Context.TargetActor.IsValid())
		Flags |= SpellContextFlags::HasTargetActor;

	if (Context.DamageCauser.IsValid())
		Flags |= SpellContextFlags::HasDamageCauser;

	if (Context.bHasHitResult)
		Flags |= SpellContextFlags::HasHitResult;

	return Flags;
}

uint8 FSpellTriggerMatcher::GetRequiredFlagsForActionType(ESpellActionType ActionType)
{
	using namespace SpellContextFlags;

	switch (ActionType)
	{
	case ESpellActionType::ConsumePierce:      return HasDamageCauser;
	case ESpellActionType::ApplyDamage:        return HasTargetASC;
	case ESpellActionType::ApplyElement:       return HasTargetASC;
	case ESpellActionType::ApplyDebuff:        return None;
	case ESpellActionType::SpawnActor:         return None;
	case ESpellActionType::ApplyGameplayEffect:return HasTargetASC | HasSourceASC;
	case ESpellActionType::EmitNiagara:        return None;
	case ESpellActionType::PlayCue:            return None;
	case ESpellActionType::Knockback:          return None;
	case ESpellActionType::DestroyActor:       return HasDamageCauser;
	case ESpellActionType::Interrupt:          return HasTargetASC;
	case ESpellActionType::None:               return None;
	// default 없음 — 새 ActionType 추가 시 여기도 반드시 채워야 컴파일됨
	}

	// 스위치가 다루지 못하는 값(캐스팅 등) 대비 필수 반환 경로. RequiredFlags가 None이라
	// 실제로는 컨텍스트 검사를 항상 통과시킨다(막지 않음).
	return None;
}
