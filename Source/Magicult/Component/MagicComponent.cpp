// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicComponent.h"
#include "GameplayTagContainer.h"
#include "Magicult/Data/MagicCircleInfo.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/Data/ElementDataTypes.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"

UMagicComponent::UMagicComponent()
{
	MagicCircle = CreateDefaultSubobject<UStaticMeshComponent>("MagicCircle");
	MagicCircle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MagicCircle->SetVisibility(false);
}

void UMagicComponent::BeginPlay()
{
	Super::BeginPlay();

	AttachMagicCircle();
	OnChangedCurrentSpellState.AddUObject(this, &UMagicComponent::HandleSpellStateChange);
}

void UMagicComponent::AttachMagicCircle()
{
	if (MagicCircle)
	{
		auto ParentComponent = GetOwner()->GetRootComponent();
		MagicCircle->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepRelativeTransform, FName("MagicCircleSocket"));
	}
}

void UMagicComponent::HandleSpellStateChange(FSpellState Current)
{
	// TODO
	// 엘리먼트 태그에 맞게 마법진 메테리얼 변경
	SetMagicCircleMaterialByTag(Current.ElementTagPair.Tag);
	// 디렉션 태그에 맞게 마법진 위치 변경
	
	// EffectTags들에 맞게 마법진 모양 변경~ 가능하다면
}


void UMagicComponent::SetSpellTag(const FGameplayTag& AbilityTag, int32 Level)
{
	if (AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Magic.Direction"))))
	{
		CurrentSpellState.DirectionTag = AbilityTag;
	}
	else if (AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Magic.Speed"))))
	{
		CurrentSpellState.SpeedTag = AbilityTag;
	}
	else if (AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Magic.Element"))))
	{
		CurrentSpellState.ElementTagPair = FSpellTagLevelPair(AbilityTag, Level);
	}
	else if (AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Magic.AmmoType"))))
	{
		CurrentSpellState.AmmoTypeTagPair = FSpellTagLevelPair(AbilityTag, Level);
	}
	else if (AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Magic.Effect"))))
	{
		CurrentSpellState.EffectTags.Add(AbilityTag, Level);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("잘못된 Tag가 들어왔습니다 %s"), *AbilityTag.ToString());
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("use magic %s"), *AbilityTag.ToString());
	OnChangedCurrentSpellState.Broadcast(CurrentSpellState);
}

void UMagicComponent::SetSpellState(const FSpellState& NewSpellState)
{
	CurrentSpellState = NewSpellState;
	OnChangedCurrentSpellState.Broadcast(CurrentSpellState);
}

void UMagicComponent::SetSpellDirectionTag(const FGameplayTag& DirectionTag)
{
	CurrentSpellState.DirectionTag = DirectionTag;
	OnChangedCurrentSpellState.Broadcast(CurrentSpellState);
}

void UMagicComponent::SetSpellSpeedTag(const FGameplayTag& SpeedTag)
{
	CurrentSpellState.SpeedTag = SpeedTag;
	OnChangedCurrentSpellState.Broadcast(CurrentSpellState);
}

void UMagicComponent::SetSpellElementTag(const FGameplayTag& ElementTag, int32 Level)
{
	CurrentSpellState.ElementTagPair = FSpellTagLevelPair(ElementTag, Level);
	OnChangedCurrentSpellState.Broadcast(CurrentSpellState);
}

void UMagicComponent::SetSpellAmmoTypeTag(const FGameplayTag& AmmoTypeTag, int32 Level)
{
	CurrentSpellState.AmmoTypeTagPair = FSpellTagLevelPair(AmmoTypeTag, Level);
	OnChangedCurrentSpellState.Broadcast(CurrentSpellState);
}

void UMagicComponent::AddSpellEffectTags(const FGameplayTag& EffectTag, int32 Level)
{
	CurrentSpellState.EffectTags.Add(EffectTag, Level);
	OnChangedCurrentSpellState.Broadcast(CurrentSpellState);
}

void UMagicComponent::SetMagicCircleMaterialByTag(const FGameplayTag& ElementTag)
{
	if (!MagicCircleMaterials && !MagicCircle) return;
	FCircleInfo Info = MagicCircleMaterials->FindMagicCircleInfoForTag(ElementTag);
	
	if (Info.ElementMaterial)
	{
		MagicCircle->SetMaterial(0, Info.ElementMaterial);
	}
}

void UMagicComponent::ActivateMagicCircle()
{
	if (MagicCircle)
	MagicCircle->SetVisibility(true);
}

void UMagicComponent::DeactivateMagicCircle()
{
	if(MagicCircle)
		MagicCircle->SetVisibility(false);
}
