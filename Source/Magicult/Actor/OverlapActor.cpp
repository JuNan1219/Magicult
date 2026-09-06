// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlapActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/Attack/OverlapDamageConfig.h"
#include "Magicult/Data/SpellTypes.h"

AOverlapActor::AOverlapActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionGuard = CreateDefaultSubobject<UCollisionGuardComponent>(TEXT("CollisionGuard"));
}

void AOverlapActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOverlapActor, SpellContext);
	DOREPLIFETIME(AOverlapActor, TriggerActionBindings);
	DOREPLIFETIME(AOverlapActor, bOverlapActive);
}

void AOverlapActor::Initialize(const FSpellContext& InContext, const TArray<FSpellTriggerActionBinding>& ParentTriggerActionBindings)
{
	SpellContext = InContext;
	SpellContext.DamageCauser = this;

	TriggerActionBindings.Append(ParentTriggerActionBindings);
}

void AOverlapActor::InitializeWithConfig_Implementation(const FSpellContext& InContext,const UOverlapDamageConfig* Config, const TArray<FSpellTriggerActionBinding>& ParentTriggerActionBindings, const FAttackSpec& InAttackSpec, bool bOverrideAttackSpec)
{
	SpellContext = InContext;
	SpellContext.DamageCauser = this;

	if (bOverrideAttackSpec)
	{
		SetAttackSpec(InAttackSpec);
	}

	TriggerActionBindings.Append(ParentTriggerActionBindings);

	SetOverlapConfig(Config);
}

void AOverlapActor::SetOverlapConfig(const UOverlapDamageConfig* InConfig)
{
	OverlapConfig = InConfig;

	if (HasActorBegunPlay())
	{
		ApplyOverlapConfig();
	}
}

void AOverlapActor::ApplyOverlapConfig_Implementation()
{
	if (!OverlapConfig)
	{
		return;
	}

	CollisionGuard->bCanHitSameTargetAgain = OverlapConfig->bCanHitSameTargetAgain;
}

void AOverlapActor::Expire()
{
	if (bExpired)
	{
		return;
	}

	bExpired = true;

	OnExpired();
}
void AOverlapActor::OnExpired()
{
	// 파괴는 여기서 직접 실행하지 않는다. Trigger_OverlapActor_Expire에 바인딩된
	// DestroyActor 액션이 RegisterPendingCleanupObject를 호출해 처리한다 —
	// 그래야 파괴가 이 트리거의 액션 배치 안에서 이뤄져 원자성이 깨지지 않는다.
	UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(this, FMagicultGameplayTags::Get().Trigger_OverlapActor_Expire, FGameplayTagContainer(), SpellContext);
}

void AOverlapActor::LifeSpanExpired()
{
	Expire();
}

void AOverlapActor::Destroyed()
{
	Super::Destroyed();
}

void AOverlapActor::PrepareCleanup_Implementation()
{
	if (bCleanupPrepared)
	{
		return;
	}

	bCleanupPrepared = true;

	// LifeSpan에 의한 재진입 방지, 타이머 초기화
	SetLifeSpan(0.f);

	UnbindOverlapGuard();

	if (UPrimitiveComponent* Collider = GetCollider())
	{
		Collider->SetGenerateOverlapEvents(false);
		Collider->SetNotifyRigidBodyCollision(false);
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorEnableCollision(false);

#if !UE_BUILD_SHIPPING
	if (bDrawDebug) DrawDebug(false);
#endif
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	// 중요:
	// SpellContext, AttackSpec, TriggerActions는 지우지 않음.
	// TriggerAction들이 DamageCauser에서 이 값을 읽을 수 있어야 함.
}

void AOverlapActor::UnbindOverlapGuard()
{
	if (CollisionGuard)
	{
		CollisionGuard->OnGuardedCollision.RemoveDynamic(
			this,
			&AOverlapActor::HandleGuardedCollision
		);

		CollisionGuard->ResetGuard();
		CollisionGuard->SetCollisionComponent(nullptr);
		CollisionGuard->Deactivate();
		CollisionGuard->SetComponentTickEnabled(false);
	}
}

void AOverlapActor::BeginPlay()
{
	Super::BeginPlay();

	if (!GetCollider())
	{
		UE_LOG(LogTemp, Warning, TEXT("콜라이더가 없습니다.%s"), *GetName());
		ensure(GetCollider());
		return;
	}
	CollisionGuard->SetCollisionComponent(GetCollider());
	CollisionGuard->OnGuardedCollision.AddDynamic(this, &AOverlapActor::HandleGuardedCollision);

	ApplyOverlapConfig();
}

UPrimitiveComponent* AOverlapActor::GetCollider_Implementation()
{
	return nullptr;
}

void AOverlapActor::DrawDebug(bool bEnabled)
{
#if !UE_BUILD_SHIPPING
	if (UPrimitiveComponent* Collider = GetCollider())
	{
		Collider->SetHiddenInGame(!bEnabled);
	}
#endif
}

void AOverlapActor::HandleGuardedCollision(const FGuardedCollisionEvent& CollisionEvent)
{
	AActor* OtherActor = CollisionEvent.OtherActor;
	UPrimitiveComponent* OtherComp = CollisionEvent.OtherComponent;

	if (!OtherActor)
	{
		return;
	}

	FGameplayTagContainer TriggerTag;

	if (OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
	{
		TriggerTag.AddTag(FMagicultGameplayTags::Get().HitType_Environment);
	}

	FSpellContext NewContext = SpellContext;

	NewContext.HitResult = CollisionEvent.HitResult;
	NewContext.bHasHitResult = CollisionEvent.bFromSweep;
	NewContext.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	NewContext.TargetActor = OtherActor;

	UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(
		this,
		FMagicultGameplayTags::Get().Trigger_Hit,
		TriggerTag,
		NewContext
	);
}

const TArray<FSpellTriggerActionBinding>& AOverlapActor::GetTriggerActionBindings() const
{
	return TriggerActionBindings;
}

void AOverlapActor::SetAttackSpec(const FAttackSpec& InAttackSpec)
{
	AttackSpec = InAttackSpec;
}

FAttackSpec AOverlapActor::GetAttackSpec() const
{
	return AttackSpec;
}

FVector AOverlapActor::GetActorVelocity() const
{
	return GetVelocity();
}

void AOverlapActor::EnableOverlap()
{
	if (HasAuthority())
	{
		bOverlapActive = true;
	}
	ApplyOverlapActive();
}

void AOverlapActor::DisableOverlap()
{
	if (HasAuthority())
	{
		bOverlapActive = false;
	}
	ApplyOverlapActive();
}

void AOverlapActor::OnRep_bOverlapActive()
{
	ApplyOverlapActive();
}

void AOverlapActor::ApplyOverlapActive()
{
	UPrimitiveComponent* Collider = GetCollider();
	if (!Collider)
	{
		return;
	}

	Collider->SetGenerateOverlapEvents(bOverlapActive);
	Collider->SetCollisionEnabled(bOverlapActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

#if !UE_BUILD_SHIPPING
	if (bDrawDebug) DrawDebug(bOverlapActive);
#endif
}
