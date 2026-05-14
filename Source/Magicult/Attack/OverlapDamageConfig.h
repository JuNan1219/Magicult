// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackInstanceConfig.h"
#include "OverlapDamageConfig.generated.h"

UENUM(BlueprintType)
enum class EOverlapShape : uint8
{
    Sphere,
    Box,
    Capsule
};

USTRUCT(BlueprintType)
struct FOverlapCollisionConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EOverlapShape Shape = EOverlapShape::Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Shape == EOverlapShape::Sphere", EditConditionHides))
    float SphereRadius = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Shape == EOverlapShape::Box", EditConditionHides))
    FVector BoxExtent = FVector(100.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Shape == EOverlapShape::Capsule", EditConditionHides))
    float CapsuleRadius = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Shape == EOverlapShape::Capsule", EditConditionHides))
    float CapsuleHalfHeight = 100.f;
};


/**
 * 
 */
UCLASS()
class MAGICULT_API UOverlapDamageConfig : public UAttackInstanceConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlap")
    FOverlapCollisionConfig CollisionConfig;

	/** 한 번 활성화 구간에서 같은 대상을 1번만 타격할지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Overlap")
	bool bCanHitSameTargetAgain = true;
};
