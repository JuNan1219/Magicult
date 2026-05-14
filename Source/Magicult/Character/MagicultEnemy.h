// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultCharacterBase.h"
#include "Magicult/UI/WidgetController/OverlayWidgetController.h"
#include "Magicult//Data/CharacterClassInfo.h"
#include "Magicult/Interface/EnemyInterface.h"
#include "MagicultEnemy.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class AMagicultAIController;
class AController;

/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultEnemy : public AMagicultCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:
	AMagicultEnemy();
	virtual void PossessedBy(AController* NewController);

	/** Combat interface */
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void Die() override;
	/** End Combat interface */
	/** Enemy interface */
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	/** End Enemy interface */

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

	UPROPERTY(BlueprintAssignable)
	FOnFloatChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnFloatChangedSignature OnMaxHealthChanged;

	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5;
protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Slime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<AMagicultAIController> MagicultAIController;
};
