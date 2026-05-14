// Fill out your copyright notice in the Description page of Project Settings.


#include "TrapButton.h"
#include "TrapActor.h"

void ATrapButton::PressButton_Implementation()
{
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
