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

	static ESpellActionRequireFlags MakeContextFlags(const FSpellContext& Context);
	
};