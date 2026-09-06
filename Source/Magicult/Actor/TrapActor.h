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
    ATrapActor();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
    // 복제된 bTrapActive에 맞춰 StartTrap/StopTrap을 호출한다. 서버/클라 공통 진입점
    void ApplyTrapActive();

    UFUNCTION()
    void OnRep_TrapActive();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAttackSpec AttackSpec;

    // 서버에서만 값을 바꾸고, 클라는 OnRep으로 같은 연출을 재생한다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_TrapActive)
    bool bTrapActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FSpellTriggerActionBinding> TriggerActionBindings;
};
