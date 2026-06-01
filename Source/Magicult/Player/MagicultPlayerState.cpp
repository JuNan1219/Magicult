// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultPlayerState.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "Magicult/Player/MagicultLobbyPlayerState.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

AMagicultPlayerState::AMagicultPlayerState()
{
	NetUpdateFrequency = 100.f;

	AbilitySystemComponent = CreateDefaultSubobject<UMagicultAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMagicultAttributeSet>("AttributeSet");
}

void AMagicultPlayerState::BeginPlay()
{
	Super::BeginPlay();

}
void AMagicultPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMagicultPlayerState, Level);
	DOREPLIFETIME(AMagicultPlayerState, PlayerIndex);
}

UAbilitySystemComponent* AMagicultPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMagicultPlayerState::InitializeStartupData()
{
	if (bIsStartupDataInitialized) return;

	check(IsValid(AbilitySystemComponent));
	check(DefaultPrimaryAttributes);
	check(DefaultSecondaryAttributes);
	check(DefaultVitalAttributes);

	const auto ContextHandle = AbilitySystemComponent->MakeEffectContext();
	auto SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultPrimaryAttributes, 1, ContextHandle);
	AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), AbilitySystemComponent);

	SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultSecondaryAttributes, 1, ContextHandle);
	AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), AbilitySystemComponent);

	SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultVitalAttributes, 1, ContextHandle);
	AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), AbilitySystemComponent);

	Cast<UMagicultAbilitySystemComponent>(AbilitySystemComponent)->UpdateAbilityStatuses(Level);

	bIsStartupDataInitialized = true;
}

void AMagicultPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);
	// SeamlessTravel 시 로비 PlayerState에서 PlayerIndex 복사
	if (const AMagicultLobbyPlayerState* LPS = Cast<AMagicultLobbyPlayerState>(PlayerState))
	{
		PlayerIndex = LPS->PlayerIndex;
	}
}

void AMagicultPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	if (AMagicultPlayerState* MPS = Cast<AMagicultPlayerState>(PlayerState))
	{
		MPS->PlayerIndex = PlayerIndex;
	}
}

void AMagicultPlayerState::OnRep_Level(int32 OldLevel)
{
}
