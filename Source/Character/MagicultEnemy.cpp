// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultEnemy.h"
#include "AbilitySystem/MagicultAbilitySystemComponent.h"
#include "AbilitySystem/MagicultAttributeSet.h"

AMagicultEnemy::AMagicultEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UMagicultAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UMagicultAttributeSet>("AttributeSet");
}
