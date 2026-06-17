// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackInstance_OverlapDamage.h"
#include "GameFramework/Actor.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/Attack/OverlapDamageConfig.h"
#include "Magicult/Actor/OverlapActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Magicult/Subsystem/CombatSubsystem.h"

void UAttackInstance_OverlapDamage::HandleEventTag_Implementation(const FGameplayTag& EventTag)
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
	if (OverlapActor.IsValid())
	{
		OverlapActor->DisableOverlap();
	}
}

void UAttackInstance_OverlapDamage::OnFinishedInternal_Implementation()
{
	if (OverlapActor.IsValid())
	{
		OverlapActor->DisableOverlap();
	}
	// TODO : 액터 삭제 트리거 액션

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

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwnerActor();

	OverlapActor = GetWorld()->SpawnActor<AOverlapActor>(
		OverlapActorClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

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

		OverlapActor->InitializeWithConfig(Context, Cast<UOverlapDamageConfig>(AttackConfig), TriggerActionBindings, AttackSpec, true);
	}
}
