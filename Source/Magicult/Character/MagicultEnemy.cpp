// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultEnemy.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"

AMagicultEnemy::AMagicultEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UMagicultAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UMagicultAttributeSet>("AttributeSet");
}
