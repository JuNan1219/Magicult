#include "AttackInstance.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/MagicultGameplayTags.h"

void UAttackInstance::Initialize(const FSpellContext& InSpellContext, const FAttackSpec& InAttackSpec, bool bOverrideAttackSpec)
{
	Context = InSpellContext;
	if (bOverrideAttackSpec)
	{
		SetAttackSpec(InAttackSpec);
	}
	FinishReason = EAttackInstanceFinishReason::None;
}

void UAttackInstance::SetAttackSpec(const FAttackSpec& InAttackSpec)
{
	AttackSpec = InAttackSpec;
}

void UAttackInstance::SetSocketTag(FGameplayTag InSocketTag)
{
	SocketTag = InSocketTag;
}

void UAttackInstance::SetInstanceConfig(UAttackInstanceConfig* InConfig)
{
	AttackConfig = InConfig;
	InitializeWithConfig();
}

UWorld* UAttackInstance::GetWorld() const
{
	if (const UObject* OuterObj = GetOuter())
	{
		return OuterObj->GetWorld();
	}

	return nullptr;
}

void UAttackInstance::Execute()
{
	if (bActive || bFinishing)
	{
		return;
	}

	bActive = true;
	bFinishing = false;
	FinishReason = EAttackInstanceFinishReason::None;

	OnExecuteStarted();
}

const TArray<FSpellTriggerActionBinding>& UAttackInstance::GetTriggerActionBindings() const
{
	return TriggerActionBindings;
}

FAttackSpec UAttackInstance::GetAttackSpec() const
{
	return AttackSpec;
}

FVector UAttackInstance::GetActorVelocity() const
{
	if (!GetOwnerActor()) return FVector::ZeroVector;
	return GetOwnerActor()->GetVelocity();
}

void UAttackInstance::OnExecuteStarted_Implementation()
{
}

void UAttackInstance::HandleEventTag_Implementation(const FGameplayTag& EventTag)
{
	if (!bActive || bFinishing)
	{
		return;
	}

	if (EventTag.MatchesTagExact(FMagicultGameplayTags::Get().Notify_Attack_End))
	{
		Finish();
	}
}

void UAttackInstance::Finish()
{
	FinishInternal(EAttackInstanceFinishReason::Normal);
}

void UAttackInstance::Cancel()
{
	FinishInternal(EAttackInstanceFinishReason::Cancelled);
}

void UAttackInstance::FinishInternal(EAttackInstanceFinishReason InReason)
{
	if (!bActive || bFinishing)
	{
		return;
	}

	bFinishing = true;
	bActive = false;
	FinishReason = InReason;

	OnFinishedInternal();
	OnFinished.Broadcast(this);
}

void UAttackInstance::OnFinishedInternal_Implementation()
{
}

void UAttackInstance::SetOwnerActor(AActor* InOwner)
{
	OwnerActor = InOwner;
}

AActor* UAttackInstance::GetOwnerActor() const
{
	return OwnerActor.Get();
}
