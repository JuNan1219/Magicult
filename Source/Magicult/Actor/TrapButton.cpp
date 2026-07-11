// Fill out your copyright notice in the Description page of Project Settings.


#include "TrapButton.h"
#include "TrapActor.h"
#include "AbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"

ATrapButton::ATrapButton()
{
    AbilitySystemComponent = CreateDefaultSubobject<UMagicultAbilitySystemComponent>("AbilitySystemComponent");
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

    AttributeSet = CreateDefaultSubobject<UMagicultAttributeSet>("AttributeSet");
}

UAbilitySystemComponent* ATrapButton::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ATrapButton::BeginPlay()
{
    Super::BeginPlay();

    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    AbilitySystemComponent->SetNumericAttributeBase(UMagicultAttributeSet::GetMaxHealthAttribute(), ButtonHealth);
    AbilitySystemComponent->SetNumericAttributeBase(UMagicultAttributeSet::GetHealthAttribute(), ButtonHealth);

    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMagicultAttributeSet::GetHealthAttribute()).AddUObject(
        this,
        &ATrapButton::HandleHealthChanged
    );
}

void ATrapButton::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] TrapButton Health %.1f -> %.1f"), *GetName(), Data.OldValue, Data.NewValue);

    if (Data.NewValue <= 0.f)
    {
        PressButton();
    }
}

void ATrapButton::PressButton_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[%s] ATrapButton::PressButton_Implementation (C++ base) reached, bOneShot=%d bPressed=%d LinkedTraps=%d"),
        *GetName(), bOneShot, bPressed, LinkedTraps.Num());

    if (bOneShot && bPressed)
    {
        return;
    }

    bPressed = true;

    for (ATrapActor* Trap : LinkedTraps)
    {
        if (IsValid(Trap))
        {
            Trap->StopTrap();
        }
    }
}
