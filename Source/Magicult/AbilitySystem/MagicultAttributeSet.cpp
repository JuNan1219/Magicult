// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultAttributeSet.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/Character/MagicultCharacterBase.h"

UMagicultAttributeSet::UMagicultAttributeSet()
{

}

void UMagicultAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Vital Attribute
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, MaxSpeed, COND_None, REPNOTIFY_Always);

	// Affinity Attribute
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, RockAffinity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, FlameAffinity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, IceAffinity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, WindAffinity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, LightningAffinity, COND_None, REPNOTIFY_Always);

	// Resistance Attribute = Affinity Attribute랑 같은 값을 가짐
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, RockResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, FlameResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, IceResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, WindResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);

	// 캐릭터별 Damage 배수
	DOREPLIFETIME_CONDITION_NOTIFY(UMagicultAttributeSet, DamageModifier, COND_None, REPNOTIFY_Always);
}

void UMagicultAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

}

void UMagicultAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UMagicultAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	// Source = effect 사용자, Target = effect 타겟(AS의 소유자)
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

	if (Props.TargetCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(Props.TargetCharacter)) return;
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
	if (Data.EvaluatedData.Attribute == GetMaxSpeedAttribute())
	{
		SetMaxSpeed(FMath::Max(0.f, GetMaxSpeed()));
	}

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Props);
	}
}
void UMagicultAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage(0.f);
	if (LocalIncomingDamage > 0.f)
	{
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

		const bool bFatal = NewHealth <= 0.f;
		if (bFatal)
		{
			// Die 처리
			ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
			if (CombatInterface)
			{
				CombatInterface->Die();
			}
		}
		else
		{
			// 피격 시 전투 타이머 GE 적용 → RecentlyDamaged 태그 부여 → 체력 리젠 일시 정지/리셋.
			// 사망 타격에는 적용하지 않는다.
			if (AMagicultCharacterBase* TargetCharacter = Cast<AMagicultCharacterBase>(Props.TargetAvatarActor))
			{
				TargetCharacter->ApplyCombatTimeout();
			}
		}

		/*
		ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCriticalHit);
		*/
	}
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

void UMagicultAttributeSet::OnRep_MaxSpeed(const FGameplayAttributeData& OldMaxSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, MaxSpeed, OldMaxSpeed);
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

void UMagicultAttributeSet::OnRep_LightningAffinity(const FGameplayAttributeData& OldLightningAffinity) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, LightningAffinity, OldLightningAffinity);
}

void UMagicultAttributeSet::OnRep_DamageModifier(const FGameplayAttributeData& OldDamageModifier) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, DamageModifier, OldDamageModifier);
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

void UMagicultAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMagicultAttributeSet, LightningResistance, OldLightningResistance);
}
