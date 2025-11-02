// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultCharacterBase.h"

AMagicultCharacterBase::AMagicultCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Weapon = CreateDefaultSubobject<UStaticMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMagicultCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

