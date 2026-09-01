// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlayWidgetController.h"
#include "GameplayEffectTypes.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/Component/MagicComponent.h"
#include "Magicult/Interface/MagicInterface.h"
#include "Magicult/UI/ViewModel/HotKeyBarViewModel.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(MagicultAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(MagicultAttributeSet->GetMaxHealth());
	OnMaxManaChanged.Broadcast(MagicultAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(MagicultAttributeSet->GetMaxMana());

	if (HotKeyBarViewModel)
	{
		HotKeyBarViewModel->BroadcastCurrent();
	}
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetMagicultAS()->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			// 풀피에서 주기 회복 GE가 적용돼도 PostGameplayEffectExecute의 Clamp로 값이
			// 그대로면(OldValue==NewValue) 굳이 UI를 다시 갱신하지 않는다.
			if (FMath::IsNearlyEqual(Data.NewValue, Data.OldValue)) return;
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetMagicultAS()->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetMagicultAS()->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetMagicultAS()->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);

	if (!MagicInfo) return;

	// ViewModel은 아래 람다들이 참조하므로 구독을 걸기 전에 만들어 둔다.
	HotKeyBarViewModel = NewObject<UHotKeyBarViewModel>(this);
	HotKeyBarViewModel->Initialize(MagicInfo, GetMagicultASC());

	FMagicultGameplayTags GameplayTags =  FMagicultGameplayTags::Get();
	AbilitySystemComponent->RegisterGameplayTagEvent(GameplayTags.Character_State_MagicMode).AddLambda([this](const FGameplayTag Tag, int32 NewCount)
		{
			FMagicInfoArray* MagicInfoArray = MagicInfo->ModeToMagics.Find(Tag);
			FMagicultGameplayTags GameplayTags = FMagicultGameplayTags::Get();
			if (MagicInfoArray)
			{
				for (auto& MagicInfo : MagicInfoArray->Magics)
				{
					MagicInfo.StatusTag = GameplayTags.Magic_Status_Unlocked;
				}
				OnTagChanged.Broadcast(Tag, *MagicInfoArray, NewCount);
			}

			HotKeyBarViewModel->HandleMagicModeChanged(NewCount > 0);
		});
	AbilitySystemComponent->RegisterGenericGameplayTagEvent().AddLambda(
		[this](const FGameplayTag Tag, int32 NewCount)
		{
			FMagicultGameplayTags GameplayTags = FMagicultGameplayTags::Get();
			// 5가지 모드 태그 전부 여기서 처리
			if (Tag.MatchesTag(GameplayTags.Character_Mode) && Tag != GameplayTags.Character_Mode)
			{
				// ModeToMagics에 없는 Character.Mode.* 태그가 붙어도 죽지 않도록 널 가드를 둔다.
				FMagicInfoArray* MagicInfoArray = MagicInfo->ModeToMagics.Find(Tag);
				if (MagicInfoArray)
				{
					for (auto& MagicInfo : MagicInfoArray->Magics)
					{
						FGameplayTag GameplayTag = GetMagicultASC()->GetStatusFromAbilityTag(MagicInfo.AbilityTag);
						if (GameplayTag.IsValid())
							MagicInfo.StatusTag = GameplayTags.Magic_Status_Unlocked;
						else
							MagicInfo.StatusTag = GameplayTags.Magic_Status_Locked;
					}
					OnTagChanged.Broadcast(Tag, *MagicInfoArray, NewCount);
				}

				HotKeyBarViewModel->HandleModeTagChanged(Tag, NewCount);
			}
		}
	);

	// 마법 설정의 마지막 단계는 Tag/GE가 아니라 MagicComponent를 직접 호출하는 구조라
	// ASC 태그 이벤트로는 못 잡는다. 소유 Pawn의 MagicComponent 델리게이트를 직접 구독해 중계한다.
	// 서버는 이 시점에 Pawn이 이미 유효하지만, 클라이언트는 PlayerState 복제와 Controller-Pawn 링크
	// 복제가 독립적이라 아직 없을 수 있다. 그래서 "지금 즉시" + "나중에(OnPossessedPawnChanged)" 둘 다
	// 시도하고, 실제 바인딩은 BindToMagicComponent의 가드가 컴포넌트당 한 번만 일어나도록 보장한다.
	if (PlayerController)
	{
		PlayerController->OnPossessedPawnChanged.AddDynamic(this, &UOverlayWidgetController::HandlePossessedPawnChanged);
		BindToMagicComponent(PlayerController->GetPawn());
	}
}

void UOverlayWidgetController::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	BindToMagicComponent(NewPawn);
}

void UOverlayWidgetController::BindToMagicComponent(APawn* OwnerPawn)
{
	if (!OwnerPawn || !OwnerPawn->Implements<UMagicInterface>()) return;

	UMagicComponent* OwnerMagicComponent = IMagicInterface::Execute_GetMagicComponent(OwnerPawn);
	if (!OwnerMagicComponent) return;

	// 즉시 호출과 델리게이트, 그리고 재포제션으로 인한 중복 발동까지 — 같은 컴포넌트에는
	// 람다를 한 번만 등록한다. 다른 컴포넌트(폰 교체)면 새로 등록된다.
	if (BoundMagicComponent.Get() == OwnerMagicComponent) return;
	BoundMagicComponent = OwnerMagicComponent;

	OwnerMagicComponent->OnChangedCurrentSpellState.AddLambda(
		[this](FSpellState NewSpellState, FGameplayTag ChangedTag)
		{
			OnSpellStateChanged.Broadcast(NewSpellState, ChangedTag);

			if (HotKeyBarViewModel)
			{
				HotKeyBarViewModel->HandleSpellTagCommitted(NewSpellState, ChangedTag);
			}
		}
	);
}

