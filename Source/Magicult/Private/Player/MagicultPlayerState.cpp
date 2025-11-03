// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultPlayerState.h"

#include "AbilitySystem/MagicultAbilitySystemComponent.h"
#include "AbilitySystem/MagicultAttributeSet.h"

AMagicultPlayerState::AMagicultPlayerState()
{
	NetUpdateFrequency = 100.f;

	AbilitySystemComponent = CreateDefaultSubobject<UMagicultAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UMagicultAttributeSet>("AttributeSet");
}
