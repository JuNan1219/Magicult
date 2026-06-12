// SpellTriggerMatcher.spec.cpp - FSpellTriggerMatcher 단위 테스트
// 실행: Session Frontend > Automation > "Magicult.SpellTriggerMatcher"
#include "Misc/AutomationTest.h"
#include "SpellTriggerMatcher.h"
#include "Magicult/Data/SpellActionTypes.h"
#include "Magicult/Data/TriggerTypes.h"
#include "GameplayTagsManager.h"

static FGameplayTag MakeTag(const FName& Name)
{
	return UGameplayTagsManager::Get().RequestGameplayTag(Name, false);
}

static FSpellTriggerEventData MakeTriggerEvent(
	FGameplayTag TriggerTag,
	FGameplayTagContainer ContextTags = FGameplayTagContainer())
{
	FSpellTriggerEventData Event;
	Event.TriggerTag         = TriggerTag;
	Event.TriggerContextTags = ContextTags;
	return Event;
}

DEFINE_SPEC(FSpellTriggerMatcherSpec,
	"Magicult.SpellTriggerMatcher",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

void FSpellTriggerMatcherSpec::Define()
{
	Describe(TEXT("TriggerTag 불일치"), [this]()
	{
		It(TEXT("바인딩 TriggerTag와 이벤트 TriggerTag가 다르면 액션을 수집하지 않는다"), [this]()
		{
			const FGameplayTag HitTag    = MakeTag("Trigger.Hit");
			const FGameplayTag ExpireTag = MakeTag("Trigger.Projectile.Expire");

			FSpellTriggerActionBinding Binding;
			Binding.Trigger.TriggerTag = HitTag;

			FSpellActionSpec Action;
			Action.ActionType = ESpellActionType::EmitNiagara;
			Binding.Actions.Add(Action);

			TArray<FSpellActionSpec> Out;
			FSpellTriggerMatcher::CollectActions({ Binding }, MakeTriggerEvent(ExpireTag), Out);

			TestTrue(TEXT("출력 액션이 비어 있어야 한다"), Out.IsEmpty());
		});
	});

	Describe(TEXT("TriggerTag 일치 / ContextTags 제한 없음"), [this]()
	{
		It(TEXT("RequiredFlags=None인 액션은 빈 Context에서도 수집된다"), [this]()
		{
			const FGameplayTag HitTag = MakeTag("Trigger.Hit");

			FSpellTriggerActionBinding Binding;
			Binding.Trigger.TriggerTag = HitTag;

			FSpellActionSpec Action;
			Action.ActionType = ESpellActionType::EmitNiagara;
			Binding.Actions.Add(Action);

			TArray<FSpellActionSpec> Out;
			FSpellTriggerMatcher::CollectActions({ Binding }, MakeTriggerEvent(HitTag), Out);

			TestEqual(TEXT("EmitNiagara 1개를 수집해야 한다"), Out.Num(), 1);
			TestEqual(TEXT("액션 타입이 EmitNiagara여야 한다"), Out[0].ActionType, ESpellActionType::EmitNiagara);
		});
	});

	Describe(TEXT("TriggerContextTags 필터"), [this]()
	{
		It(TEXT("이벤트에 바인딩의 ContextTags가 모두 포함된 경우에만 수집된다"), [this]()
		{
			const FGameplayTag HitTag    = MakeTag("Trigger.Hit");
			const FGameplayTag RadialTag = MakeTag("AttackType.Radial");

			FSpellTriggerActionBinding Binding;
			Binding.Trigger.TriggerTag = HitTag;
			Binding.Trigger.TriggerContextTags.AddTag(RadialTag);

			FSpellActionSpec Action;
			Action.ActionType = ESpellActionType::EmitNiagara;
			Binding.Actions.Add(Action);

			// 케이스 A: Radial 태그 없음 -> 수집 안 됨
			{
				TArray<FSpellActionSpec> Out;
				FSpellTriggerMatcher::CollectActions({ Binding }, MakeTriggerEvent(HitTag), Out);
				TestTrue(TEXT("ContextTag 없으면 액션 없음"), Out.IsEmpty());
			}

			// 케이스 B: Radial 태그 있음 -> 수집됨
			{
				FGameplayTagContainer ContextTags;
				ContextTags.AddTag(RadialTag);

				TArray<FSpellActionSpec> Out;
				FSpellTriggerMatcher::CollectActions({ Binding }, MakeTriggerEvent(HitTag, ContextTags), Out);
				TestEqual(TEXT("ContextTag 있으면 액션 수집"), Out.Num(), 1);
			}
		});
	});

	Describe(TEXT("RequiredFlags 미충족"), [this]()
	{
		It(TEXT("TargetASC가 없는 Context에서 ApplyDamage는 수집되지 않는다"), [this]()
		{
			const FGameplayTag HitTag = MakeTag("Trigger.Hit");

			FSpellTriggerActionBinding Binding;
			Binding.Trigger.TriggerTag = HitTag;

			FSpellActionSpec DamageAction;
			DamageAction.ActionType = ESpellActionType::ApplyDamage; // HasTargetASC 필요
			Binding.Actions.Add(DamageAction);

			FSpellActionSpec NiagaraAction;
			NiagaraAction.ActionType = ESpellActionType::EmitNiagara; // RequiredFlags=None
			Binding.Actions.Add(NiagaraAction);

			TArray<FSpellActionSpec> Out;
			FSpellTriggerMatcher::CollectActions({ Binding }, MakeTriggerEvent(HitTag), Out);

			TestEqual(TEXT("Niagara만 1개 수집 (Damage는 필터됨)"), Out.Num(), 1);
			TestEqual(TEXT("수집된 액션은 EmitNiagara여야 한다"), Out[0].ActionType, ESpellActionType::EmitNiagara);
		});
	});

	Describe(TEXT("복수 바인딩"), [this]()
	{
		It(TEXT("같은 TriggerTag를 가진 여러 바인딩의 액션이 모두 합산된다"), [this]()
		{
			const FGameplayTag HitTag = MakeTag("Trigger.Hit");

			auto MakeBinding = [&](ESpellActionType Type) -> FSpellTriggerActionBinding
			{
				FSpellTriggerActionBinding B;
				B.Trigger.TriggerTag = HitTag;
				FSpellActionSpec A;
				A.ActionType = Type;
				B.Actions.Add(A);
				return B;
			};

			TArray<FSpellTriggerActionBinding> Bindings = {
				MakeBinding(ESpellActionType::EmitNiagara),
				MakeBinding(ESpellActionType::PlayCue),
			};

			TArray<FSpellActionSpec> Out;
			FSpellTriggerMatcher::CollectActions(Bindings, MakeTriggerEvent(HitTag), Out);

			TestEqual(TEXT("두 바인딩의 액션이 모두 수집된다"), Out.Num(), 2);
		});
	});
}
