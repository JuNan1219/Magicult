// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultProjectile.h"
#include "MagicultProjectile_Box.generated.h"

class UBoxComponent;
struct FSpellColliderSpec;

/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultProjectile_Box : public AMagicultProjectile
{
	GENERATED_BODY()
	
public:
	AMagicultProjectile_Box();

	virtual void ApplyColliderSpec(const FSpellColliderSpec& ColliderSpec) override;

	virtual UPrimitiveComponent* GetCollider() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> BoxCollider;
};
