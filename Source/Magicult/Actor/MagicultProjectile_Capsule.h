// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultProjectile.h"
#include "MagicultProjectile_Capsule.generated.h"

class UCapsuleComponent;
struct FSpellColliderSpec;

/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultProjectile_Capsule : public AMagicultProjectile
{
	GENERATED_BODY()
public:
	AMagicultProjectile_Capsule();
	virtual void ApplyColliderSpec(const FSpellColliderSpec& ColliderSpec) override;

	virtual UPrimitiveComponent* GetCollider() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> CapsuleCollider;

};
