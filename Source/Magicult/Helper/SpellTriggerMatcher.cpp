#include "SpellTriggerMatcher.h"

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

	const ESpellActionRequireFlags CurrentFlags = MakeContextFlags(EventData.SpellContext);

	for (const FSpellActionSpec& ActionSpec : Binding.Actions)
	{
		const ESpellActionRequireFlags RequiredFlags = static_cast<ESpellActionRequireFlags>(ActionSpec.RequiredContextFlags);

		if (!EnumHasAllFlags(CurrentFlags, RequiredFlags))
		{
			continue;
		}
		OutActions.Add(ActionSpec);
	}

	return !OutActions.IsEmpty();
}

ESpellActionRequireFlags FSpellTriggerMatcher::MakeContextFlags(const FSpellContext& Context)
{
	ESpellActionRequireFlags Flags = ESpellActionRequireFlags::None;

	if (Context.SourceAbilitySystemComponent.IsValid())
		Flags |= ESpellActionRequireFlags::HasSourceASC;

	if (Context.TargetAbilitySystemComponent.IsValid())
		Flags |= ESpellActionRequireFlags::HasTargetASC;

	if (Context.SourceActor.IsValid())
		Flags |= ESpellActionRequireFlags::HasSourceActor;

	if (Context.TargetActor.IsValid())
		Flags |= ESpellActionRequireFlags::HasTargetActor;

	if (Context.DamageCauser.IsValid())
		Flags |= ESpellActionRequireFlags::HasDamageCauser;

	if (Context.bHasHitResult)
		Flags |= ESpellActionRequireFlags::HasHitResult;

	return Flags;
}
