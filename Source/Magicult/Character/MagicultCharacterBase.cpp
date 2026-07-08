// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Magicult/Component/MagicComponent.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/Player/MagicultPlayerState.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Magicult/MagicultGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "GameplayEffectTypes.h"

AMagicultCharacterBase::AMagicultCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MagicComponent = CreateDefaultSubobject<UMagicComponent>("MagicComponent");
	MagicComponent->SetupAttachment(RootComponent);

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

	// 모든 채널 기본값 Ignore
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// 지형(WorldStatic) 충돌 — 이동/착지용
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	// Pawn끼리 겹침 방지
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	GetMesh()->SetGenerateOverlapEvents(true);

	// 모든 채널 기본값 Ignore
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// WorldDynamic(박스, 통, 움직이는 물체 등)과 충돌
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Block);
}

UAbilitySystemComponent* AMagicultCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMagicultCharacterBase::BeginPlay()
{
	Super::BeginPlay();

}

void AMagicultCharacterBase::InitializeDefaultAttributes() const
{

}

void AMagicultCharacterBase::AddCharacterAbilities()
{
	UMagicultAbilitySystemComponent* MagicultASC = CastChecked<UMagicultAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority()) return;

	MagicultASC->AddCharacterAbilities(StartupAbilities);
	MagicultASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

void AMagicultCharacterBase::InitAbilityActorInfo()
{
	// AttributeSet && AbilitySystemComponent 를 자식에서 초기화 후 호출
	if (AttributeSet && AbilitySystemComponent)
	{
		UMagicultAttributeSet* MagicultAS = Cast<UMagicultAttributeSet>(AttributeSet);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MagicultAS->GetMaxSpeedAttribute()).AddUObject(this, &AMagicultCharacterBase::OnMaxSpeedChanged);

		// 이미 값이 들어와 있을 수 있으니 현재값 1회 적용
		ApplyCurrentMaxSpeed();

		AbilitySystemComponent->RegisterGameplayTagEvent(FMagicultGameplayTags::Get().Debuff_Stunned, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,
			&AMagicultCharacterBase::StunTagChanged
		);
	}
}

void AMagicultCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bStunned = NewCount > 0;
	OnStunnedChanged(bStunned);
}

void AMagicultCharacterBase::OnStunnedChanged(bool bNewStunned)
{
}

UAnimMontage* AMagicultCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

FTransform AMagicultCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	const FMagicultGameplayTags& GameplayTags = FMagicultGameplayTags::Get();
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Weapon) && IsValid(Weapon))
	{
		return Weapon->GetSocketTransform(WeaponTipSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_LHand))
	{
		return GetMesh()->GetSocketTransform(LeftHandSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_RHand))
	{
		return GetMesh()->GetSocketTransform(RightHandSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Head))
	{
		return GetMesh()->GetSocketTransform(HeadSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Magic))
	{
		return GetMesh()->GetSocketTransform(MagicSocketName);
	}
	return FTransform();
}

bool AMagicultCharacterBase::AttachActorToSocket_Implementation(const FGameplayTag& SocketTag, AActor* Target)
{
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("타겟이 유효하지 않습니다. %s"), *GetName());
		return false;
	}

	if (!SocketTag.IsValid())
	{
		return Target->AttachToComponent(
			GetRootComponent(),
			FAttachmentTransformRules::KeepRelativeTransform,
			NAME_None
		);
	}

	FName SocketName;
	const FMagicultGameplayTags& GameplayTags = FMagicultGameplayTags::Get();
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_Weapon) && IsValid(Weapon))
		SocketName = WeaponTipSocketName;
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_LHand))
		SocketName = LeftHandSocketName;
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_RHand))
		SocketName = RightHandSocketName;
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_Head))
		SocketName = HeadSocketName;
	if (SocketTag.MatchesTagExact(GameplayTags.CombatSocket_Magic))
		SocketName = MagicSocketName;

	return Target->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName
	);
}

void AMagicultCharacterBase::Die()
{
	if (!HasAuthority()) return;

	// 사망 시 모든 디버프 이펙트 제거 (태그 복제 → 클라이언트 비주얼 자동 정리)
	if (AbilitySystemComponent)
	{
		const FMagicultGameplayTags& GameplayTags = FMagicultGameplayTags::Get();
		FGameplayTagContainer DebuffTags;
		DebuffTags.AddTag(GameplayTags.Debuff_Burn);
		DebuffTags.AddTag(GameplayTags.Debuff_Freeze);
		DebuffTags.AddTag(GameplayTags.Debuff_Stunned);
		DebuffTags.AddTag(GameplayTags.Debuff_Ragdoll);
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(DebuffTags);
	}

	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	MulticastHandleDeath();
}

bool AMagicultCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

FGameplayTag AMagicultCharacterBase::GetTeamTag_Implementation() const
{
	return TeamTag;
}

void AMagicultCharacterBase::MulticastHandleDeath_Implementation()
{
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	// 카메라(SpringArm) 분리: 래그돌 메시를 따라 카메라가 날아가지 않도록 사망 지점에 고정한다.
	// 모든 머신에서 돌지만 카메라는 플레이어에만 있어 적에겐 no-op(FindComponentByClass null).
	if (USpringArmComponent* SpringArm = FindComponentByClass<USpringArmComponent>())
	{
		SpringArm->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	}

	Dissolve();
	bDead = true;
}

void AMagicultCharacterBase::MulticastApplyRagdollImpulse_Implementation(FVector Impulse, FName BoneName)
{
	// 래그돌이 아직 활성화 안 됐으면(사망 멀티캐스트 처리 전) 스킵. 보통은 사망 RPC가 먼저 도착한다.
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (MeshComp->IsSimulatingPhysics())
		{
			MeshComp->AddImpulse(Impulse, BoneName, true);
		}
	}
}

const TArray<FAttackDefinition> AMagicultCharacterBase::GetAttackDefinitions_Implementation(const FGameplayTag& InputTag)
{
	for (const auto& AttackDefinitions : TaggedAttackDefinitions)
	{
		if (AttackDefinitions.TriggerTag.MatchesTagExact(InputTag))
		{
			return AttackDefinitions.Attacks;
		}
	}

	return TArray<FAttackDefinition>();
}

UMagicComponent* AMagicultCharacterBase::GetMagicComponent_Implementation()
{
	return MagicComponent;
}

void AMagicultCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	const auto ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	const auto SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AMagicultCharacterBase::ApplyHealthRegenEffect()
{
	if (!HasAuthority()) return;
	if (!bEnableHealthRegen || !HealthRegenEffect) return;
	if (!IsValid(GetAbilitySystemComponent())) return;

	ApplyEffectToSelf(HealthRegenEffect, 1.f);
}

void AMagicultCharacterBase::ApplyCombatTimeout()
{
	if (!HasAuthority()) return;
	if (!bEnableHealthRegen || !CombatTimeoutEffect) return;
	if (!IsValid(GetAbilitySystemComponent())) return;

	ApplyEffectToSelf(CombatTimeoutEffect, 1.f);
}

void AMagicultCharacterBase::Dissolve()
{
	if (IsValid(DissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMatInst);

		StartDissolveTimeline(DynamicMatInst);
	}
	if (IsValid(WeaponDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMatInst);

		StartWeaponDissolveTimeline(DynamicMatInst);
	}
}

void AMagicultCharacterBase::ApplyMaxSpeed_Implementation(float InMaxSpeed)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = FMath::Max(0.f, InMaxSpeed);
	}
}

void AMagicultCharacterBase::OnMaxSpeedChanged(const FOnAttributeChangeData& Data)
{
	ApplyMaxSpeed(Data.NewValue);
}

void AMagicultCharacterBase::ApplyCurrentMaxSpeed()
{
	if (!AttributeSet) return;
	UMagicultAttributeSet* MagicultAS = Cast<UMagicultAttributeSet>(AttributeSet);
	if (!MagicultAS) return;
	ApplyMaxSpeed(MagicultAS->GetMaxSpeed());
}
