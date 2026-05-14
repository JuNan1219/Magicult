// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultProjectile.h"
#include "MagicultProjectile_Sphere.generated.h"

class USphereComponent;
struct FSpellColliderSpec;
/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultProjectile_Sphere : public AMagicultProjectile
{
	GENERATED_BODY()
public:
	AMagicultProjectile_Sphere();
	virtual void ApplyColliderSpec(const FSpellColliderSpec& ColliderSpec) override;
	virtual UPrimitiveComponent* GetCollider() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> SphereCollider;
};
