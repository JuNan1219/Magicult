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
	UpdateMagicCircleSpeed(Current.SpeedTag);
	UpdateMagicCircleDirection(Current.DirectionTag);
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
	if (!MagicCircleMaterials || !MagicCircle) return;
	FCircleInfo Info = MagicCircleMaterials->FindMagicCircleInfoForTag(ElementTag);
	DynamicMaterial = UMaterialInstanceDynamic::Create(Info.ElementMaterial, this);
	MagicCircle->SetMaterial(0, DynamicMaterial);
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

void UMagicComponent::UpdateMagicCircleSpeed(const FGameplayTag& SpeedTag)
{
	if (!DynamicMaterial) return;

	const FMagicultGameplayTags& Tags = FMagicultGameplayTags::Get();

	static const TMap<FGameplayTag, float> SpeedMap =
	{
		{ Tags.Magic_Speed_0, 0.f },
		{ Tags.Magic_Speed_1, 0.2f },
		{ Tags.Magic_Speed_2, 0.4f },
		{ Tags.Magic_Speed_3, 0.6f },
		{ Tags.Magic_Speed_4, 0.8f },
	};

	const float* Found = SpeedMap.Find(SpeedTag);
	DynamicMaterial->SetScalarParameterValue(FName("Speed"), Found ? *Found : 0.f);
}

void UMagicComponent::UpdateMagicCircleDirection(const FGameplayTag& DirectionTag)
{
	if (!MagicCircle) return;

	const FMagicultGameplayTags& Tags = FMagicultGameplayTags::Get();

	static const TMap<FGameplayTag, float> DirectionMap =
	{
		{ Tags.Magic_Direction_0, 0.f },
		{ Tags.Magic_Direction_1, 15.f },
		{ Tags.Magic_Direction_2, 30.f },
		{ Tags.Magic_Direction_3, 45.f },
		{ Tags.Magic_Direction_4, 60.f },
	};

	const float* Found = DirectionMap.Find(DirectionTag);
	const float Pitch = Found ? *Found : 0.f;
	MagicCircle->SetRelativeRotation(FRotator(Pitch, 0.f, 0.f));
}

void UMagicComponent::ClearSpellState()
{
	CurrentSpellState.Clear();
	OnChangedCurrentSpellState.Broadcast(CurrentSpellState);
}
