// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnProjectileAttackInstance.h"
#include "Magicult/Component/MagicComponent.h"
#include "Magicult/Interface/MagicInterface.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/Actor/MagicultProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/Subsystem/CombatResolveSubsystem.h"
#include "Magicult/Subsystem/CombatSubsystem.h"
#include "Magicult/MagicultGameplayTags.h"

void USpawnProjectileAttackInstance::HandleEventTag_Implementation(const FGameplayTag& EventTag)
{
    Super::HandleEventTag_Implementation(EventTag);
    
    if (EventTag.IsValid() && EventTag.MatchesTagExact(FMagicultGameplayTags::Get().Notify_Attack_Spawn))
    {
        AActor* SourceActor = Context.SourceActor.Get();
        if (SourceActor && SourceActor->Implements<UMagicInterface>())
        {
            UMagicComponent* MagicComponent = IMagicInterface::Execute_GetMagicComponent(SourceActor);
            SpawnMagicProjectile(MagicComponent->CurrentSpellState);
        }
    }
}

bool USpawnProjectileAttackInstance::ConvertTagsToProjectileData(const FSpellState& CurrentSpellState, FSpellContext& OutSpellContext, FResolvedSpellProjectileResult& OutResolvedSpellProjectileResult)
{
    if (!CurrentSpellState.DirectionTag.IsValid() ||
        !CurrentSpellState.SpeedTag.IsValid() ||
        !CurrentSpellState.ElementTagPair.Tag.IsValid() ||
        !CurrentSpellState.AmmoTypeTagPair.Tag.IsValid())
        return false;


    OutResolvedSpellProjectileResult = GetWorld()->GetSubsystem<UCombatResolveSubsystem>()->ResolveProjectile(OutSpellContext, CurrentSpellState);

    return true;
}
bool USpawnProjectileAttackInstance::CheckSpellState(const FSpellState& CurrentSpellState)
{
    return (CurrentSpellState.DirectionTag.IsValid() &&
        CurrentSpellState.SpeedTag.IsValid() &&
        CurrentSpellState.ElementTagPair.Tag.IsValid() &&
        CurrentSpellState.AmmoTypeTagPair.Tag.IsValid());
}

void USpawnProjectileAttackInstance::SpawnMagicProjectile(const FSpellState& CurrentSpellState)
{
    UCombatSubsystem* CombatSubsystem = GetWorld()->GetSubsystem<UCombatSubsystem>();
    if (!CombatSubsystem) return;

    if (!CheckSpellState(CurrentSpellState))
    {
        UE_LOG(LogTemp, Warning, TEXT("%s"), *FString("The magic setting is incomplete. Please complete the spell."));

        // - 내 자신이 데미지 입는 Trigger
        FSpellTriggerEventData NewTriggerEventData;

        FSpellContext SelfDamageSpellContext = Context;
        SelfDamageSpellContext.TargetAbilitySystemComponent = Context.SourceAbilitySystemComponent;
        SelfDamageSpellContext.TargetActor = Context.SourceActor;
        SelfDamageSpellContext.DamageCauser = this;

        NewTriggerEventData.SpellContext = SelfDamageSpellContext;
        NewTriggerEventData.TriggerTag = FMagicultGameplayTags::Get().Trigger_Projectile_SpawnFailure;

        CombatSubsystem->Enqueue(NewTriggerEventData);
        return;
    }

    FTransform SocketTransform = FTransform::Identity;
    if(Context.SourceActor.IsValid() && Context.SourceActor->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
        SocketTransform = ICombatInterface::Execute_GetCombatSocketLocation(Context.SourceActor.Get(), SocketTag);

    CombatSubsystem->SpawnProjectile(Context, CurrentSpellState, SocketTransform, AttackSpec.BaseDamage);
}
