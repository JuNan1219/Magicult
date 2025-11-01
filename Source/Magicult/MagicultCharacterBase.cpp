// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultCharacterBase.h"

AMagicultCharacterBase::AMagicultCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AMagicultCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}


void AMagicultCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

