// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultProjectile_Capsule.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"
#include "Magicult/Data/ProjectileTypes.h"

AMagicultProjectile_Capsule::AMagicultProjectile_Capsule()
{
    CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollider"));
    SetRootComponent(CapsuleCollider);

    NiagaraComponent->SetupAttachment(RootComponent);
}

void AMagicultProjectile_Capsule::ApplyColliderSpec(const FSpellColliderSpec& ColliderSpec)
{
	if (!CapsuleCollider)
	{
		return;
	}

	if (ColliderSpec.Shape != ESpellColliderShape::Capsule)
	{
		UE_LOG(LogTemp, Warning, TEXT("Capsule projectile received non-capsule collider spec."));
		return;
	}

	CapsuleCollider->SetCapsuleSize(ColliderSpec.CapsuleRadius, ColliderSpec.CapsuleHalfHeight, true);
}

UPrimitiveComponent* AMagicultProjectile_Capsule::GetCollider()
{
	return CapsuleCollider;
}

