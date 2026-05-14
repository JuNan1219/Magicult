// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackContextProvider.h"

// Add default functionality here for any IAttackContextProvider functions that are not pure virtual.

FAttackSpec IAttackContextProvider::GetAttackSpec() const
{
	return FAttackSpec();
}

FVector IAttackContextProvider::GetActorVelocity() const
{
	return FVector::ZeroVector;
}