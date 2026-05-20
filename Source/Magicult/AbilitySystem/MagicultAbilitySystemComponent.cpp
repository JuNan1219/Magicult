// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultAbilitySystemComponent.h"
#include "Magicult/AbilitySystem/Abilities/MagicultGameplayAbility.h"
#include "Magicult/AbilitySystem/Abilities/MagicultPassiveAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Magicult/Data/MagicInfo.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"

void UMagicultAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UMagicultAbilitySystemComponent::EffectApplied);
}

void UMagicultAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	if (bStartupAbilitiesGiven) return;
	for (const auto AbilityClass : StartupAbilities)
	{
		auto AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const auto MagicultAbility = Cast<UMagicultGameplayAbility>(AbilitySpec.Ability))
		{
			GiveAbility(AbilitySpec);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("일반 Abilty Give 시도 %s"), AbilitySpec.Ability.GetFName());
		}
	}
	bStartupAbilitiesGiven = true;
}

void UMagicultAbilitySystemComponent::CancelAbilitiesWithTag(FGameplayTag Tag)
{
	FGameplayTagContainer Tags(Tag);
	CancelAbilities(&Tags);
}

void UMagicultAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const auto AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (Cast<UMagicultPassiveAbility>(AbilitySpec.Ability))
		{
			FGameplayAbilitySpecHandle Handle = GiveAbility(AbilitySpec);
			TryActivateAbility(Handle);
		}
	}
}

void UMagicultAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	UE_LOG(LogTemp, Warning, TEXT("%s"), *InputTag.ToString());

	FGameplayEventData Payload;
	Payload.EventTag = InputTag;
	Payload.Instigator = GetAvatarActor();

	//이 함수가 트리거가 일치하는 어빌리티를 찾아 'ActivateAbilityFromEvent'를 실행합니다.
	//TryActivateAbility를 따로 호출할 필요가 없습니다.
	HandleGameplayEvent(InputTag, &Payload);
}

void UMagicultAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	
	for (auto& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				if (TryActivateAbility(AbilitySpec.Handle))
				{
					return;
				}
			}
		}
	}
}

void UMagicultAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (auto& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void UMagicultAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UMagicultAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FMagicultGameplayTags::Get().Magic))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag UMagicultAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag InputTag : AbilitySpec.DynamicAbilityTags)
	{
		if (InputTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return InputTag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UMagicultAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Magic.Status"))))
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UMagicultAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayAbilitySpec* UMagicultAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

void UMagicultAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UMagicInfo* MagicInfo = UMagicultAbilitySystemLibrary::GetMagicInfo(GetAvatarActor());
	if (!MagicInfo) return;

	for (const FMagicultMagicInfo& Info : MagicInfo->MagicInformation)
	{
		if (!Info.AbilityTag.IsValid()) continue;
		if (Level < Info.LevelRequirement) continue;
		if (GetSpecFromAbilityTag(Info.AbilityTag) != nullptr) continue;
		if (FMagicultGameplayTags::Get().Magic_Type_Active.MatchesTagExact(Info.AbilityType))
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);

			AbilitySpec.DynamicAbilityTags.AddTag(FMagicultGameplayTags::Get().Magic_Status_Unlocked);
			MarkAbilitySpecDirty(AbilitySpec);
			FGameplayAbilitySpecHandle SpecHandle = GiveAbility(AbilitySpec);
			ClientUpdateAbilityStatus(Info.AbilityTag, FMagicultGameplayTags::Get().Magic_Status_Unlocked, 1);
		}
		else if(FMagicultGameplayTags::Get().Magic_Type_Passive.MatchesTagExact(Info.AbilityType))
		{
			// 패시브 버튼 활성화
			ClientUpdateAbilityStatus(Info.AbilityTag, FMagicultGameplayTags::Get().Magic_Status_Eligible, 1);
		}
	}
	
}

void UMagicultAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}


void UMagicultAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
#if !UE_BUILD_SHIPPING
	GEngine->AddOnScreenDebugMessage(1, 8.f, FColor::Black, FString("Effect Applied!"));
#endif
}
