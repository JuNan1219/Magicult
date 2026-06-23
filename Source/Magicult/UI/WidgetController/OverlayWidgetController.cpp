// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlayWidgetController.h"
#include "GameplayEffectTypes.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(MagicultAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(MagicultAttributeSet->GetMaxHealth());
	OnMaxManaChanged.Broadcast(MagicultAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(MagicultAttributeSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetMagicultAS()->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
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
		});
	AbilitySystemComponent->RegisterGenericGameplayTagEvent().AddLambda(
		[this](const FGameplayTag Tag, int32 NewCount)
		{
			FMagicultGameplayTags GameplayTags = FMagicultGameplayTags::Get();
			// 5가지 모드 태그 전부 여기서 처리
			if (Tag.MatchesTag(GameplayTags.Character_Mode) && Tag != GameplayTags.Character_Mode)
			{
				FMagicInfoArray* MagicInfoArray = MagicInfo->ModeToMagics.Find(Tag);
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
		}
	);
}

void UOverlayWidgetController::SetModeInputBlocked(bool bBlocked)
{
	if (!AbilitySystemComponent) return;

	// 로컬 전용(비복제) 태그: 이 클라이언트의 모드 선택 어빌리티 발동만 막는 UX 게이트라서
	// 서버에 알릴 필요가 없다. 서버 권한 GE로 적용되는 모드 태그 흐름과는 무관하다.
	const FGameplayTag BlockTag = FMagicultGameplayTags::Get().Character_State_ModeInputBlocked;
	if (bBlocked)
	{
		AbilitySystemComponent->AddLooseGameplayTag(BlockTag);
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlockTag);
	}
}
