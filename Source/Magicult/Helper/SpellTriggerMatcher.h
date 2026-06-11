#pragma once

#include "Magicult/Data/ProjectileTypes.h"
#include "Magicult/Data/SpellTypes.h"

class FSpellTriggerMatcher
{
public:
	static void CollectActions(
		const TArray<FSpellTriggerActionBinding>& Bindings,
		const FSpellTriggerEventData& EventData,
		TArray<FSpellActionSpec>& OutActions);

private:
	static bool TryCollectActions(
		const FSpellTriggerActionBinding& Binding,
		const FSpellTriggerEventData& EventData,
		TArray<FSpellActionSpec>& OutActions);

	static uint8 MakeContextFlags(const FSpellContext& Context);
	// ActionType별 기본 RequiredContextFlags 반환. 새 ActionType 추가 시 반드시 케이스 추가.
	static uint8 GetRequiredFlagsForActionType(ESpellActionType ActionType);
};