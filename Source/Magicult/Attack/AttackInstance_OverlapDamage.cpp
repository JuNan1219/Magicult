// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackInstance_OverlapDamage.h"
#include "GameFramework/Actor.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/Attack/OverlapDamageConfig.h"
#include "Magicult/Actor/OverlapActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Magicult/Subsystem/CombatSubsystem.h"

void UAttackInstance_OverlapDamage::HandleEventTag_Implementation(FGameplayTag EventTag)
{
	if (!bActive || bFinishing)
	{
		return;
	}

	const FMagicultGameplayTags& Tags = FMagicultGameplayTags::Get();

	Super::HandleEventTag_Implementation(EventTag);

	if (!bActive || bFinishing)
	{
		return;
	}

	if (OverlapActor.IsValid() && EventTag.IsValid() && EventTag.MatchesTagExact(Tags.Notify_Attack_HitboxOn))
	{
		OverlapActor->EnableOverlap();
		return;
	}
}

void UAttackInstance_OverlapDamage::OnExecuteStarted_Implementation()
{
	Super::OnExecuteStarted_Implementation();

	CreateOverlapCollider();
}

void UAttackInstance_OverlapDamage::OnFinishedInternal_Implementation()
{
	if (OverlapActor.IsValid())
	{
		OverlapActor->DisableOverlap();
		OverlapActor->Expire();
	}

	Super::OnFinishedInternal_Implementation();
}

void UAttackInstance_OverlapDamage::CreateOverlapCollider()
{
	if (!OverlapActorClass)
	{
		return;
	}

	AActor* Owner = GetOwnerActor();
	if (!Owner)
	{
		return;
	}

	USceneComponent* AttachComponent = Owner->GetRootComponent();
	if (!AttachComponent)
	{
		return;
	}

	if (!GetWorld()) return;

	OverlapActor = GetWorld()->SpawnActorDeferred<AOverlapActor>(
		OverlapActorClass,
		FTransform(),
		GetOwnerActor()
	);
	if (AttackConfig)
	{
		const UOverlapDamageConfig* OverlapConfig = Cast<UOverlapDamageConfig>(AttackConfig);
		if (!OverlapConfig)
		{
			UE_LOG(LogTemp, Warning, TEXT("AttackConfig가 UOverlapDamageConfig 타입이 아닙니다. Owner : %s"), *GetOwnerActor()->GetName());
		}
		OverlapActor->InitializeWithConfig(Context, OverlapConfig, TriggerActionBindings, AttackSpec, true);
	}

	OverlapActor->FinishSpawning(FTransform());

	if (!OverlapActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor 생성에 실패했습니다."));
		return;
	}
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwnerActor()))
	{
		if(!ICombatInterface::Execute_AttachActorToSocket(GetOwnerActor(), SocketTag, OverlapActor.Get()))
		{
			UE_LOG(LogTemp, Warning, TEXT("액터 Attach에 실패했습니다. Socekt : %s, Owner : %s"), *SocketTag.ToString(), *GetOwnerActor()->GetName());
			return;
		}
	}
}
