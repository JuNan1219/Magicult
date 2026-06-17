// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackInstance.h"
#include "Components/SphereComponent.h"
#include "AttackInstance_OverlapDamage.generated.h"

class UOverlapDamageConfig;
class AOverlapActor;
/**
 * 
 */
UCLASS()
class MAGICULT_API UAttackInstance_OverlapDamage : public UAttackInstance
{
	GENERATED_BODY()
	
public:
	virtual void HandleEventTag_Implementation(const FGameplayTag& EventTag) override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AOverlapActor> OverlapActorClass;

protected:
	virtual void OnExecuteStarted_Implementation() override;
	virtual void OnFinishedInternal_Implementation() override;

	/** Execute 시 콜라이더 생성 */
	void CreateOverlapCollider();

private:
	UPROPERTY()
	TWeakObjectPtr<AOverlapActor> OverlapActor;
};
