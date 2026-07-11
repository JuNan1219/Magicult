// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "TrapButton.generated.h"

class ATrapActor;
class UAbilitySystemComponent;
class UMagicultAttributeSet;
struct FOnAttributeChangeData;

UCLASS()
class MAGICULT_API ATrapButton : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
    ATrapButton();

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void PressButton();

    virtual void PressButton_Implementation();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TObjectPtr<ATrapActor>> LinkedTraps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOneShot = true;

    bool bPressed = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY()
    TObjectPtr<UMagicultAttributeSet> AttributeSet;

    // 버튼 체력. 데미지로 0이 되면 눌린 것으로 취급한다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap Button")
    float ButtonHealth = 1.f;

private:
    void HandleHealthChanged(const FOnAttributeChangeData& Data);
};
