// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorActor.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

ADoorActor::ADoorActor()
{
	bReplicates = true;

	DoorBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorBox"));
	SetRootComponent(DoorBox);
	DoorBox->SetBoxExtent(FVector(50.f, 150.f, 150.f));
	DoorBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DoorBox->SetCollisionResponseToAllChannels(ECR_Block);
}

void ADoorActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		bClosed = bStartClosed;
	}
	ApplyDoorState();
}

void ADoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADoorActor, bClosed);
}

void ADoorActor::CloseDoor_Implementation()
{
	if (!HasAuthority()) return;
	bClosed = true;
	ApplyDoorState();
}

void ADoorActor::OpenDoor_Implementation()
{
	if (!HasAuthority()) return;
	bClosed = false;
	ApplyDoorState();
}

void ADoorActor::OnRep_Closed()
{
	ApplyDoorState();
}

void ADoorActor::ApplyDoorState()
{
	DoorBox->SetCollisionEnabled(bClosed ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}
