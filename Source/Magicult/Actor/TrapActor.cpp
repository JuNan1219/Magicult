// Fill out your copyright notice in the Description page of Project Settings.


#include "TrapActor.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Net/UnrealNetwork.h"

ATrapActor::ATrapActor()
{
    bReplicates = true;

    // 동작 연출은 각 머신이 타임라인으로 직접 재생한다. 위치까지 복제하면 서버 값이 클라 연출을 덮어쓴다
    SetReplicateMovement(false);
}

void ATrapActor::BeginPlay()
{
    Super::BeginPlay();

    // 클라는 BeginPlay 전에 OnRep이 올 수 있으므로 복제된 현재 상태를 그대로 반영한다
    ApplyTrapActive();
}

void ATrapActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATrapActor, bTrapActive);
}

void ATrapActor::ApplyTrapActive()
{
    if (bTrapActive)
    {
        StartTrap();
    }
    else
    {
        StopTrap();
    }
}

void ATrapActor::OnRep_TrapActive()
{
    ApplyTrapActive();
}

void ATrapActor::StopTrap_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] ATrapActor::StopTrap_Implementation (C++ base) reached"), *GetName());

    // 서버에서만 상태를 확정한다. 클라는 이미 복제된 값을 받고 여기로 들어온 것이라 덮어쓰면 안 된다
    if (HasAuthority())
    {
        bTrapActive = false;
    }

    // 데미지 판정 끄기
    SetActorEnableCollision(false);
}

void ATrapActor::StartTrap_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] ATrapActor::StartTrap_Implementation (C++ base) reached"), *GetName());

    if (HasAuthority())
    {
        bTrapActive = true;
    }

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
