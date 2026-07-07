// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicultBoss.h"
#include "Magicult/AI/Boss/BossCombatComponent.h"

AMagicultBoss::AMagicultBoss()
{
	BossCombatComponent = CreateDefaultSubobject<UBossCombatComponent>("BossCombatComponent");
}
