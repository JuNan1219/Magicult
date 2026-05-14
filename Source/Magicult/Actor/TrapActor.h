// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/Interface/AttackContextProvider.h"
#include "Magicult/Interface/ActionSourceInterface.h"
#include "TrapActor.generated.h"

UCLASS()
class MAGICULT_API ATrapActor : public AActor, public IAttackContextProvider, public IActionSourceInterface
{
	GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void StopTrap();

    virtual void StopTrap_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void StartTrap();

    virtual void StartTrap_Implementation();

    /*
    * Interface Start
    */
    virtual FAttackSpec GetAttackSpec() const override;
    virtual FVector GetActorVelocity() const override;

    virtual const TArray<FSpellTriggerActionBinding>& GetTriggerActionBindings() const override;
    /*
    * Interface end
    */
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAttackSpec AttackSpec;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTrapActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FSpellTriggerActionBinding> TriggerActionBindings;
};
