// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultEnemy.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"

AMagicultEnemy::AMagicultEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UMagicultAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UMagicultAttributeSet>("AttributeSet");
}

void AMagicultEnemy::BeginPlay()
{
	Super::BeginPlay();

	InitAbilityActorInfo();
}

void AMagicultEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UMagicultAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}
