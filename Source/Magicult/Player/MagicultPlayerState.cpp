// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultPlayerState.h"

#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"

AMagicultPlayerState::AMagicultPlayerState()
{
	NetUpdateFrequency = 100.f;

	AbilitySystemComponent = CreateDefaultSubobject<UMagicultAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMagicultAttributeSet>("AttributeSet");
}

UAbilitySystemComponent* AMagicultPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
