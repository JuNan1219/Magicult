// Fill out your copyright notice in the Description page of Project Settings.


#include "TrapActor.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"

void ATrapActor::BeginPlay()
{
    Super::BeginPlay();

    StartTrap();
}

void ATrapActor::StopTrap_Implementation()
{
    bTrapActive = false;

    // 데미지 판정 끄기
    SetActorEnableCollision(false);
}

void ATrapActor::StartTrap_Implementation()
{
    bTrapActive = true;

    SetActorEnableCollision(true);
    SetActorTickEnabled(true);
}

FAttackSpec ATrapActor::GetAttackSpec() const
{
    return AttackSpec;
}

FVector ATrapActor::GetActorVelocity() const
{
    return FVector();
}

const TArray<FSpellTriggerActionBinding>& ATrapActor::GetTriggerActionBindings() const
{
    return TriggerActionBindings;
}
