// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"

AMagicultCharacterBase::AMagicultCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Weapon = CreateDefaultSubobject<UStaticMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UAbilitySystemComponent* AMagicultCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMagicultCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMagicultCharacterBase::InitAbilityActorInfo()
{

}

void AMagicultCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	const auto ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	const auto SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AMagicultCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void AMagicultCharacterBase::AddCharacterAbilities()
{
	UMagicultAbilitySystemComponent* MagicultASC = CastChecked<UMagicultAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority()) return;
	
	MagicultASC->AddCharacterAbilities(StartupAbilities);
}
