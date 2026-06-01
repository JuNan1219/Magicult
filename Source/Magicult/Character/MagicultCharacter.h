// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultCharacterBase.h"
#include "MagicultCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
/**
 *
 */
UCLASS()
class MAGICULT_API AMagicultCharacter : public AMagicultCharacterBase
{
	GENERATED_BODY()
public:
	AMagicultCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** Combat Interface */
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual FName GetDefaultKnockbackBone() const override;
	virtual void Die() override;
	/** end Combat Interface */

protected:
	virtual void InitializeDefaultAttributes() const;

	// 인덱스 0 = 1P 파란색, 인덱스 1 = 2P 빨간색
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
	TObjectPtr<UMaterialInterface> Player1Material;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
	TObjectPtr<UMaterialInterface> Player2Material;

private:
	virtual void InitAbilityActorInfo() override;

	// PlayerState로부터 받아 복제됨
	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex)
	int32 PlayerIndex = 0;

	UFUNCTION()
	void OnRep_PlayerIndex();

	void ApplyPlayerColor();
};
