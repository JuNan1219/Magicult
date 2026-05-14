// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultProjectile_Sphere.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Magicult/Data/ProjectileTypes.h"

AMagicultProjectile_Sphere::AMagicultProjectile_Sphere()
{
	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollider"));
	SetRootComponent(SphereCollider);

	NiagaraComponent->SetupAttachment(RootComponent);
}

void AMagicultProjectile_Sphere::ApplyColliderSpec(const FSpellColliderSpec& ColliderSpec)
{
	if (!SphereCollider)
	{
		return;
	}

	if (ColliderSpec.Shape != ESpellColliderShape::Sphere)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sphere projectile received non-sphere collider spec."));
		return;
	}

	SphereCollider->SetSphereRadius(ColliderSpec.SphereRadius, true);
}

UPrimitiveComponent* AMagicultProjectile_Sphere::GetCollider()
{
	return SphereCollider;
}
