// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AMagicultCharacter::AMagicultCharacter()
{
	CameraArm = CreateDefaultSubobject<USpringArmComponent>("CameraArm");
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->TargetArmLength = 1200.f;
	CameraArm->SocketOffset = FVector(0.f, 0.f, 900.f);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(CameraArm);
}

void AMagicultCharacter::BeginPlay()
{
	Super::BeginPlay();
}
