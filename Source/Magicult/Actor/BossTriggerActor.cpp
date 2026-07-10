// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTriggerActor.h"
#include "Components/BoxComponent.h"
#include "DoorActor.h"
#include "Magicult/Character/MagicultEnemy.h"
#include "GameFramework/Pawn.h"

ABossTriggerActor::ABossTriggerActor()
{
	bReplicates = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ABossTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	if (bActivateOnPlayerOverlap)
	{
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABossTriggerActor::OnTriggerBeginOverlap);
	}

	RemainingToClear = 0;
	for (AMagicultEnemy* Monster : MonstersToClear)
	{
		if (!IsValid(Monster)) continue;
		++RemainingToClear;
		Monster->OnDeath.AddDynamic(this, &ABossTriggerActor::OnTrackedMonsterDied);
	}
}

void ABossTriggerActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn || !OtherPawn->IsPlayerControlled()) return;

	Activate();
}

void ABossTriggerActor::OnTrackedMonsterDied(AMagicultEnemy* DeadEnemy)
{
	--RemainingToClear;
	if (RemainingToClear <= 0)
	{
		Activate();
	}
}

void ABossTriggerActor::Activate()
{
	if (!HasAuthority()) return;
	if (bTriggered) return;
	bTriggered = true;

	for (ADoorActor* Door : DoorsToClose)
	{
		if (IsValid(Door))
		{
			Door->CloseDoor();
		}
	}

	for (ADoorActor* Door : DoorsToOpen)
	{
		if (IsValid(Door))
		{
			Door->OpenDoor();
		}
	}

	for (AMagicultEnemy* Enemy : EnemiesToActivate)
	{
		if (IsValid(Enemy))
		{
			Enemy->ActivateCombat();
		}
	}
}
