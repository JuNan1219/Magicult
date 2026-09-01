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
#include "Animation/AnimInstance.h"

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

	CacheReviveDefaults();
}

void AMagicultCharacterBase::CacheReviveDefaults()
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		DefaultMeshRelativeTransform = MeshComp->GetRelativeTransform();
	}
	if (IsValid(Weapon))
	{
		DefaultWeaponRelativeTransform = Weapon->GetRelativeTransform();
		DefaultWeaponAttachSocket = Weapon->GetAttachSocketName();
		DefaultWeaponMaterial = Weapon->GetMaterial(0);
	}
	// SpringArm은 플레이어 BP에만 있는 컴포넌트다(몬스터에선 null → no-op).
	if (USpringArmComponent* SpringArm = FindComponentByClass<USpringArmComponent>())
	{
		DefaultSpringArmParent = SpringArm->GetAttachParent();
		DefaultSpringArmRelativeTransform = SpringArm->GetRelativeTransform();
	}
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
	// 같은 프레임에 치명 데미지가 여러 번 들어와도 사망 처리는 1회만 한다(부활 타이머 중복 방지).
	if (bDead) return;

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

		// 마법 모드도 사망 시 취소한다. DA_InterruptPolicy(피격 인터럽트)와 동일한 대상 태그를 써서
		// 같은 캐스케이드(GA_MagicMode::EndAbility -> CancelAbilitiesWithTag(Character.Mode))를 태운다 —
		// ASC는 PlayerState 소유라 리스폰 이후에도 살아남으므로, 여기서 안 끊으면 마법 모드/모드
		// 선택 상태가 리스폰 뒤까지 새어나간다.
		FGameplayTagContainer MagicModeTag(GameplayTags.Character_State_MagicMode);
		AbilitySystemComponent->CancelAbilities(&MagicModeTag);
	}

	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));

	// 부활은 죽은 자리에서 이뤄진다. 래그돌 메시가 아니라 캡슐 위치를 기준으로 잡는다.
	DeathLocation = GetActorLocation();
	DeathRotation = GetActorRotation();

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

	// 죽은 뒤에도 (보이지 않는) 캡슐이 입력으로 밀려다니지 않도록 이동을 정지한다.
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	Dissolve();
	bDead = true;
}

void AMagicultCharacterBase::Revive()
{
	if (!HasAuthority()) return;
	if (!bDead) return;

	// 사망 후 캡슐이 밀렸을 수 있으므로 사망 시점 위치로 되돌린다.
	SetActorLocationAndRotation(DeathLocation, DeathRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// bDead 해제가 먼저다. 그래야 이어지는 바이탈 리셋이 AttributeSet의 사망 게이트에 막히지 않는다.
	MulticastHandleRevive();

	ResetVitalAttributes();
}

void AMagicultCharacterBase::MulticastHandleRevive_Implementation()
{
	// MulticastHandleDeath가 바꾼 것들을 역순으로 되돌린다.
	// 물리 시뮬 중인 컴포넌트는 어태치가 무시되므로 SimulatePhysics(false)를 먼저 끈다.
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetSimulatePhysics(false);
		MeshComp->SetEnableGravity(false);

		MeshComp->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MeshComp->SetRelativeTransform(DefaultMeshRelativeTransform);

		// 생성자 기본값 복구(사망 시 PhysicsOnly + WorldStatic Block으로 바뀌어 있다).
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

		// 히트리액트 등 사망 직전 몽타주 잔재 정리
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.1f);
		}
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (IsValid(Weapon))
	{
		Weapon->SetSimulatePhysics(false);
		Weapon->SetEnableGravity(false);
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, DefaultWeaponAttachSocket);
		Weapon->SetRelativeTransform(DefaultWeaponRelativeTransform);
		Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 사망 시 분리했던 카메라를 다시 캐릭터에 붙인다(플레이어 전용, 몬스터는 no-op).
	if (USpringArmComponent* SpringArm = FindComponentByClass<USpringArmComponent>())
	{
		USceneComponent* AttachParent = IsValid(DefaultSpringArmParent) ? DefaultSpringArmParent.Get() : GetRootComponent();
		SpringArm->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);
		SpringArm->SetRelativeTransform(DefaultSpringArmRelativeTransform);
	}

	RestoreDefaultMaterials();

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	bDead = false;
}

void AMagicultCharacterBase::RestoreDefaultMaterials()
{
	// Dissolve()가 MID로 교체한 머티리얼을 원본으로 되돌린다(디졸브 미설정이면 애초에 교체가 없어 그대로 덮어써도 무해).
	if (IsValid(Weapon) && DefaultWeaponMaterial)
	{
		Weapon->SetMaterial(0, DefaultWeaponMaterial);
	}
}

void AMagicultCharacterBase::ResetVitalAttributes()
{
	if (!HasAuthority()) return;

	UMagicultAttributeSet* MagicultAS = Cast<UMagicultAttributeSet>(AttributeSet);
	if (!MagicultAS) return;

	MagicultAS->SetHealth(MagicultAS->GetMaxHealth());
	MagicultAS->SetMana(MagicultAS->GetMaxMana());
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
