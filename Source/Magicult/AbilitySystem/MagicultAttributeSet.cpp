// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultAttributeSet.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"

UMagicultAttributeSet::UMagicultAttributeSet()
{

}

void UMagicultAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	// Affinity Attribute
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, RockAffinity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, FlameAffinity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, IceAffinity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, WindAffinity, COND_None, REPNOTIFY_Always);

	// Resistance Attribute = Affinity Attribute¶û °°Àº °ªÀ» °¡Áü
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, RockResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, FlameResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, IceResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, WindResistance, COND_None, REPNOTIFY_Always);
}

void UMagicultAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

}
void UMagicultAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	// Source = effect »ç¿ëÀÚ, Target = effect Å¸°Ù(ASÀÇ ¼ÒÀ¯ÀÚ)
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void UMagicultAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

}

void UMagicultAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, Health, OldHealth);
}

void UMagicultAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, MaxHealth, OldMaxHealth);
}

void UMagicultAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, Mana, OldMana);
}

void UMagicultAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, MaxMana, OldMaxMana);
}

void UMagicultAttributeSet::OnRep_RockAffinity(const FGameplayAttributeData& OldRockAffinity) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, RockAffinity, OldRockAffinity);
}

void UMagicultAttributeSet::OnRep_FlameAffinity(const FGameplayAttributeData& OldFlameAffinity) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, FlameAffinity, OldFlameAffinity);
}

void UMagicultAttributeSet::OnRep_IceAffinity(const FGameplayAttributeData& OldIceAffinity) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, IceAffinity, OldIceAffinity);
}

void UMagicultAttributeSet::OnRep_WindAffinity(const FGameplayAttributeData& OldWindAffinity) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, WindAffinity, OldWindAffinity);
}

void UMagicultAttributeSet::OnRep_RockResistance(const FGameplayAttributeData& OldRockResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, RockResistance, OldRockResistance);
}

void UMagicultAttributeSet::OnRep_FlameResistance(const FGameplayAttributeData& OldFlameResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, FlameResistance, OldFlameResistance);
}

void UMagicultAttributeSet::OnRep_IceResistance(const FGameplayAttributeData& OldIceResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, IceResistance, OldIceResistance);
}

void UMagicultAttributeSet::OnRep_WindResistance(const FGameplayAttributeData& OldWindResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, WindResistance, OldWindResistance);
}
