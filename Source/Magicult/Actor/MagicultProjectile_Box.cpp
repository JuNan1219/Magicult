// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultProjectile_Box.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Magicult/Data/ProjectileTypes.h"

AMagicultProjectile_Box::AMagicultProjectile_Box()
{
    BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
    SetRootComponent(BoxCollider);

    NiagaraComponent->SetupAttachment(BoxCollider);
}

void AMagicultProjectile_Box::ApplyColliderSpec(const FSpellColliderSpec& ColliderSpec)
{
	if (!BoxCollider)
	{
		return;
	}

	if (ColliderSpec.Shape != ESpellColliderShape::Box)
	{
		UE_LOG(LogTemp, Warning, TEXT("Box projectile received non-box collider spec."));
		return;
	}

	BoxCollider->SetBoxExtent(ColliderSpec.BoxHalfExtent, true);
}

UPrimitiveComponent* AMagicultProjectile_Box::GetCollider()
{
	return BoxCollider;
}

