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
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ABossTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABossTriggerActor::OnTriggerBeginOverlap);
	}
}

void ABossTriggerActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bTriggered) return;

	const APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn || !OtherPawn->IsPlayerControlled()) return;

	bTriggered = true;

	for (ADoorActor* Door : DoorsToClose)
	{
		if (IsValid(Door))
		{
			Door->CloseDoor();
		}
	}

	for (AMagicultEnemy* Boss : BossesToActivate)
	{
		if (IsValid(Boss))
		{
			Boss->ActivateCombat();
		}
	}
}
