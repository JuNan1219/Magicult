// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/Data/ProjectileTypes.h"
#include "Magicult/Subsystem/CombatSubsystem.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
// Sets default values
AMagicultProjectile::AMagicultProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	OverlapGuard = CreateDefaultSubobject<UOverlapTriggerGuardComponent>(TEXT("OverlapGuard"));
}

void AMagicultProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMagicultProjectile, SpellContext);
	DOREPLIFETIME(AMagicultProjectile, TriggerActions);
	DOREPLIFETIME(AMagicultProjectile, ProjectileClientInitData);
}

void AMagicultProjectile::LifeSpanExpired()
{
	Expire();
}

void AMagicultProjectile::Destroyed()
{
	Super::Destroyed();
}
const TArray<FSpellTriggerActionBinding>& AMagicultProjectile::GetTriggerActionBindings() const
{
	return TriggerActions;
}

void AMagicultProjectile::OnRep_ProjectileClientInitData()
{
	InitializeProjectile();
}

void AMagicultProjectile::OnRep_TriggerActions()
{
}

void AMagicultProjectile::SetProjectileSpawnData(const FSpellContext& InSpellContext, const FResolvedSpellProjectileResult& InResolvedSpellProjectileResult)
{
	if (!HasAuthority())
	{
		return;
	}

	SpellContext = InSpellContext;
	SpellContext.DamageCauser = this;
	ResolvedProjectileResult = InResolvedSpellProjectileResult;

	const FResolvedSpellProjectileSpec& Spec = ResolvedProjectileResult.Spec;

	// 서버/클라 공통으로 필요한 초기화 데이터
	ProjectileClientInitData.bValid = true;
	ProjectileClientInitData.LaunchDirection = Spec.LaunchDirection.GetSafeNormal();
	ProjectileClientInitData.InitialSpeed = Spec.InitialSpeed;
	ProjectileClientInitData.ProjectileNiagara = Spec.ProjectileNiagara;
	ProjectileClientInitData.ProjectileCollider = Spec.ProjectileCollider;
	ProjectileClientInitData.ColliderEventPolicy = Spec.CollisionEventPolicy;

	AttackSpec.BaseDamage = Spec.BaseDamage;
	AttackSpec.ElementTag = Spec.ElementTag;
	AttackSpec.ElementRatio = Spec.ElementRatio;
	AttackSpec.bUseVelocityDamage = true;
	AttackSpec.DamageModifier = Spec.DamageModifier;

	bFriendlyFire = Spec.bFriendlyFire;

	ProjectileClientInitData.SizeScale = Spec.SizeScale;
	ProjectileClientInitData.GravityScale = Spec.GravityScale;
	ProjectileClientInitData.LifeSpanMultiplier = Spec.LifeSpanMultiplier;
	ProjectileClientInitData.bFriendlyFire = Spec.bFriendlyFire;
	ProjectileClientInitData.bTracking = Spec.bTracking;
	ProjectileClientInitData.TrackingSearchRadius = Spec.TrackingSearchRadius;
	ProjectileClientInitData.HomingAccelerationMagnitude = Spec.HomingAccelerationMagnitude;

	TriggerActions = Spec.TriggerActionBindings;

	InitializeProjectile();

	ForceNetUpdate();
}

void AMagicultProjectile::InitializeProjectile()
{
	if (bProjectileInitialized)
	{
		return;
	}

	if (!ProjectileClientInitData.bValid)
	{
		return;
	}

	UPrimitiveComponent* Collider = GetCollider();
	if (!Collider)
	{
		UE_LOG(LogTemp, Warning, TEXT("콜라이더가 없습니다 %s"), *GetName());
		return;
	}

	bProjectileInitialized = true;
	SpellContext.DamageCauser = this;

	OverlapGuard->SetCollisionComponent(Collider);
	ApplyColliderSpec(ProjectileClientInitData.ProjectileCollider);
	if (NiagaraComponent)
	{
		NiagaraComponent->SetAsset(ProjectileClientInitData.ProjectileNiagara);

		if (ProjectileClientInitData.ProjectileNiagara)
		{
			NiagaraComponent->Activate(true);
		}
	}

	if (ProjectileMovement)
	{
		const FVector LaunchDirection = ProjectileClientInitData.LaunchDirection.GetSafeNormal();

		ProjectileMovement->InitialSpeed = ProjectileClientInitData.InitialSpeed;
		ProjectileMovement->Velocity = LaunchDirection * ProjectileClientInitData.InitialSpeed;
		ProjectileMovement->ProjectileGravityScale = ProjectileClientInitData.GravityScale;
		ProjectileMovement->Activate(true);
	}

	if (!FMath::IsNearlyEqual(ProjectileClientInitData.SizeScale, 1.0f))
	{
		SetActorScale3D(GetActorScale3D() * ProjectileClientInitData.SizeScale);
	}

	if (!FMath::IsNearlyEqual(ProjectileClientInitData.LifeSpanMultiplier, 1.0f))
	{
		LifeSpan *= ProjectileClientInitData.LifeSpanMultiplier;
		SetLifeSpan(LifeSpan);
	}

	bFriendlyFire = ProjectileClientInitData.bFriendlyFire;

	if (ProjectileClientInitData.bTracking && HasAuthority() && ProjectileMovement)
	{
		TArray<AActor*> OverlappingActors;
		UKismetSystemLibrary::SphereOverlapActors(
			this,
			GetActorLocation(),
			ProjectileClientInitData.TrackingSearchRadius,
			TArray<TEnumAsByte<EObjectTypeQuery>>(),
			nullptr,
			TArray<AActor*>{ GetOwner() },
			OverlappingActors
		);

		float MinDist = MAX_FLT;
		AActor* NearestEnemy = nullptr;
		for (AActor* Actor : OverlappingActors)
		{
			AActor* SourceAvatar = SpellContext.SourceAbilitySystemComponent.IsValid()
				? SpellContext.SourceAbilitySystemComponent->GetAvatarActor()
				: nullptr;
			if (!UMagicultAbilitySystemLibrary::IsNotFriend(SourceAvatar, Actor)) continue;

			const float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
			if (Dist < MinDist)
			{
				MinDist = Dist;
				NearestEnemy = Actor;
			}
		}

		if (NearestEnemy)
		{
			ProjectileMovement->HomingTargetComponent = NearestEnemy->GetRootComponent();
			ProjectileMovement->HomingAccelerationMagnitude = ProjectileClientInitData.HomingAccelerationMagnitude;
			ProjectileMovement->bIsHomingProjectile = true;
		}
	}

	BindCollisionPolicy(ProjectileClientInitData.ColliderEventPolicy);
	ConfigureCollisionForPolicy(ProjectileClientInitData.ColliderEventPolicy);
}

UPrimitiveComponent* AMagicultProjectile::GetCollider()
{
	return nullptr;
}

void AMagicultProjectile::ApplyColliderSpec(const FSpellColliderSpec& ColliderSpec)
{

}

// Called when the game starts or when spawned
void AMagicultProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSpan);
	UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(this, FMagicultGameplayTags::Get().Trigger_Projectile_Spawn, FGameplayTagContainer(), SpellContext);
}

void AMagicultProjectile::Expire()
{
	if (bExpired)
	{
		return;
	}
	bExpired = true;

	OnExpired();
}

void AMagicultProjectile::OnExpired()
{
	UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(this, FMagicultGameplayTags::Get().Trigger_Projectile_Expire, FGameplayTagContainer(), SpellContext);

	if (UCombatSubsystem* CombatSubsystem = GetWorld()->GetSubsystem<UCombatSubsystem>())
	{
		CombatSubsystem->RegisterPendingCleanupObject(this);
	}
}

void AMagicultProjectile::UnbindCollisionEvents()
{
	UPrimitiveComponent* Collider = GetCollider();

	if (OverlapGuard)
	{
		OverlapGuard->OnGuardedOverlap.RemoveDynamic(
			this,
			&AMagicultProjectile::HandleGuardedOverlap
		);

		OverlapGuard->ResetGuard();
		OverlapGuard->SetCollisionComponent(nullptr);
		OverlapGuard->Deactivate();
		OverlapGuard->SetComponentTickEnabled(false);
	}

	if (Collider)
	{
		Collider->OnComponentHit.RemoveDynamic(
			this,
			&AMagicultProjectile::HandleHit
		);
	}
}

bool AMagicultProjectile::IsValidImpact(AActor* OtherActor)
{
	if (SpellContext.SourceAbilitySystemComponent == nullptr) return false;
	AActor* SourceAvatarActor = SpellContext.SourceAbilitySystemComponent->GetAvatarActor();
	if (SourceAvatarActor == OtherActor) return false;
	if (!bFriendlyFire && !UMagicultAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return false;

	return true;
}

void AMagicultProjectile::BindCollisionPolicy(ESpellCollisionEventPolicy Policy)
{
	UPrimitiveComponent* Collider = GetCollider();

	if (!Collider)
	{
		return;
	}

	switch (Policy)
	{
	case ESpellCollisionEventPolicy::None:
		break;

	case ESpellCollisionEventPolicy::OverlapOnly:
		OverlapGuard->OnGuardedOverlap.AddDynamic(this, &AMagicultProjectile::HandleGuardedOverlap);
		break;

	case ESpellCollisionEventPolicy::HitOnly:
	case ESpellCollisionEventPolicy::Bounce:
		Collider->OnComponentHit.AddDynamic(this, &ThisClass::HandleHit);
		break;

	case ESpellCollisionEventPolicy::OverlapAndHit:
		OverlapGuard->OnGuardedOverlap.AddDynamic(this, &AMagicultProjectile::HandleGuardedOverlap);
		Collider->OnComponentHit.AddDynamic(this, &ThisClass::HandleHit);
		break;
	default:
		break;
	}
}

void AMagicultProjectile::ConfigureCollisionForPolicy(ESpellCollisionEventPolicy Policy)
{

	UPrimitiveComponent* Collider = GetCollider();
	if (!Collider)
	{
		return;
	}

	// 기본값 초기화
	Collider->SetGenerateOverlapEvents(false);

	// Hit 이벤트 발생용
	Collider->SetNotifyRigidBodyCollision(false);

	switch (Policy)
	{
	case ESpellCollisionEventPolicy::OverlapOnly:
	case ESpellCollisionEventPolicy::OverlapAndHit:
		Collider->SetGenerateOverlapEvents(true);
		break;

	default:
		break;
	}

	switch (Policy)
	{
	case ESpellCollisionEventPolicy::HitOnly:
	case ESpellCollisionEventPolicy::OverlapAndHit:
	case ESpellCollisionEventPolicy::Bounce:
		Collider->SetNotifyRigidBodyCollision(true);
		break;

	default:
		break;
	}

	if (ProjectileMovement)
	{
		// Bounce 정책일 때만 bounce 활성화
		ProjectileMovement->bShouldBounce = (Policy == ESpellCollisionEventPolicy::Bounce);
	}
}

void AMagicultProjectile::HandleGuardedOverlap(const FGuardedOverlapEvent& OverlapEvent)
{
	AActor* OtherActor = OverlapEvent.OtherActor;
	UPrimitiveComponent* OtherComp = OverlapEvent.OtherComponent;

	if (!OtherActor || !IsValidImpact(OtherActor))
	{
		return;
	}

	FGameplayTagContainer TriggerTag;

	if (OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
	{
		TriggerTag.AddTag(FMagicultGameplayTags::Get().HitType_Environment);
	}

	FSpellContext NewContext = SpellContext;

	NewContext.HitResult = OverlapEvent.SweepResult;
	NewContext.bHasHitResult = OverlapEvent.bFromSweep;
	NewContext.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	NewContext.TargetActor = OtherActor;

	UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(
		this,
		FMagicultGameplayTags::Get().Trigger_Hit,
		TriggerTag,
		NewContext
	);
}

void AMagicultProjectile::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	FGameplayTagContainer TriggerTag;
	if (OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
		TriggerTag.AddTag(FMagicultGameplayTags::Get().HitType_Environment);

	FSpellContext NewContext = SpellContext;
	NewContext.HitResult = Hit;
	NewContext.bHasHitResult = true;
	NewContext.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	NewContext.TargetActor = OtherActor;

	UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(this, FMagicultGameplayTags::Get().Trigger_Hit, TriggerTag, NewContext);
}

FAttackSpec AMagicultProjectile::GetAttackSpec() const
{
	return AttackSpec;
}

FVector AMagicultProjectile::GetActorVelocity() const
{
	return GetVelocity();
}

void AMagicultProjectile::PrepareCleanup_Implementation()
{
	if (bCleanupPrepared)
	{
		return;
	}

	bCleanupPrepared = true;

	// 더 이상 LifeSpan으로 다시 Expire/Destroy 흐름 타지 않게 정리
	SetLifeSpan(0.f);

	// 새 충돌/트리거 방지
	UnbindCollisionEvents();

	UPrimitiveComponent* Collider = GetCollider();

	if (Collider)
	{
		Collider->SetGenerateOverlapEvents(false);
		Collider->SetNotifyRigidBodyCollision(false);
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SetActorEnableCollision(false);

	// 이동 중단
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	// 시각적으로 제거
	if (NiagaraComponent)
	{
		NiagaraComponent->Deactivate();
		NiagaraComponent->SetVisibility(false, true);
	}

	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	// 중요:
	// 여기서 SpellContext, AttackSpec, TriggerActions, ResolvedProjectileResult는 지우지 않음.
	// TriggerAction들이 DamageCauser에서 값을 꺼내 쓸 수 있어야 함.
}
