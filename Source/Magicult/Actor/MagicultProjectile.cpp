// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/Data/ProjectileTypes.h"
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
	CollisionGuard = CreateDefaultSubobject<UCollisionGuardComponent>(TEXT("CollisionGuard"));
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
	ProjectileClientInitData.Bounciness = Spec.Bounciness;
	ProjectileClientInitData.RehitCooldown = Spec.RehitCooldown;

	AttackSpec.BaseDamage = Spec.BaseDamage;
	AttackSpec.ElementTag = Spec.ElementTag;
	AttackSpec.ElementRatio = Spec.ElementRatio;
	AttackSpec.bUseVelocityDamage = true;
	AttackSpec.DamageModifier = Spec.DamageModifier;

	AttackSpec.bTargetEnemiesOnly = Spec.bTargetEnemiesOnly;

	ProjectileClientInitData.SizeScale = Spec.SizeScale;
	ProjectileClientInitData.GravityScale = Spec.GravityScale;
	ProjectileClientInitData.LifeSpanMultiplier = Spec.LifeSpanMultiplier;
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

	CollisionGuard->SetCollisionComponent(Collider);
	CollisionGuard->RehitCooldown = ProjectileClientInitData.RehitCooldown;
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


	if (ProjectileClientInitData.bTracking && HasAuthority() && ProjectileMovement)
	{
		TArray<AActor*> OverlappingActors;
		UKismetSystemLibrary::SphereOverlapActors(
			this,
			GetActorLocation(),
			ProjectileClientInitData.TrackingSearchRadius,
			TArray<TEnumAsByte<EObjectTypeQuery>>{ EObjectTypeQuery::ObjectTypeQuery3 },
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
	// 파괴는 여기서 직접 실행하지 않는다. Trigger_Projectile_Expire에 바인딩된
	// DestroyActor 액션이 RegisterPendingCleanupObject를 호출해 처리한다 —
	// 그래야 파괴가 이 트리거의 액션 배치 안에서 이뤄져 원자성이 깨지지 않는다.
	UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(this, FMagicultGameplayTags::Get().Trigger_Projectile_Expire, FGameplayTagContainer(), SpellContext);
}

void AMagicultProjectile::UnbindCollisionEvents()
{
	if (CollisionGuard)
	{
		CollisionGuard->OnGuardedCollision.RemoveDynamic(
			this,
			&AMagicultProjectile::HandleGuardedCollision
		);

		CollisionGuard->ResetGuard();
		CollisionGuard->SetCollisionComponent(nullptr);
		CollisionGuard->Deactivate();
		CollisionGuard->SetComponentTickEnabled(false);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->OnProjectileBounce.RemoveDynamic(
			this,
			&AMagicultProjectile::HandleProjectileBounce
		);
	}
}

bool AMagicultProjectile::IsValidImpact(AActor* SourceAvatarActor, AActor* OtherActor) const
{
	if (!OtherActor) return false;
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
		CollisionGuard->OnGuardedCollision.AddDynamic(this, &AMagicultProjectile::HandleGuardedCollision);
		break;

	case ESpellCollisionEventPolicy::HitOnly:
		CollisionGuard->OnGuardedCollision.AddDynamic(this, &AMagicultProjectile::HandleGuardedCollision);
		break;

	case ESpellCollisionEventPolicy::Bounce:
		// 물리 튕김 자체(OnProjectileBounce)는 raw 히트 노이즈가 그대로 필요하므로 안 건드린다.
		// 데미지 트리거만 CollisionGuard를 거쳐 쿨다운이 적용되게 한다 — 안 그러면 한 번 튕길 때
		// 노이즈로 발생하는 여러 OnComponentHit마다 Trigger_Hit이 중복으로 나간다.
		CollisionGuard->OnGuardedCollision.AddDynamic(this, &AMagicultProjectile::HandleGuardedCollision);
		if (ProjectileMovement)
			ProjectileMovement->OnProjectileBounce.AddDynamic(this, &ThisClass::HandleProjectileBounce);
		break;

	case ESpellCollisionEventPolicy::OverlapAndHit:
		CollisionGuard->OnGuardedCollision.AddDynamic(this, &AMagicultProjectile::HandleGuardedCollision);
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
		Collider->SetNotifyRigidBodyCollision(true);
		Collider->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Collider->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		break;

	case ESpellCollisionEventPolicy::Bounce:
		Collider->SetNotifyRigidBodyCollision(true);
		Collider->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Collider->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		Collider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		break;

	default:
		break;
	}

	if (ProjectileMovement)
	{
		const bool bBounce = (Policy == ESpellCollisionEventPolicy::Bounce);
		ProjectileMovement->bShouldBounce = bBounce;
		// TODO : 무중력 상태에서 어색함
		//ProjectileMovement->BounceVelocityStopSimulatingThreshold = bBounce ? 0.f : 0.f;
		if (bBounce)
			ProjectileMovement->Bounciness = ProjectileClientInitData.Bounciness;
	}
}

void AMagicultProjectile::HandleGuardedCollision(const FGuardedCollisionEvent& CollisionEvent)
{
	AActor* OtherActor = CollisionEvent.OtherActor;
	UPrimitiveComponent* OtherComp = CollisionEvent.OtherComponent;

	AActor* SourceAvatar = SpellContext.SourceAbilitySystemComponent.IsValid()
		? SpellContext.SourceAbilitySystemComponent->GetAvatarActor() : nullptr;
	if (!OtherActor || !IsValidImpact(SourceAvatar, OtherActor))
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

void AMagicultProjectile::HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (!HasAuthority()) return;

	FSpellRuntimeState& RuntimeState = ResolvedProjectileResult.RuntimeState;
	RuntimeState.RemainingBounceCount = FMath::Max(0, RuntimeState.RemainingBounceCount - 1);

	if (RuntimeState.RemainingBounceCount <= 0)
	{
		UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(
			this,
			FMagicultGameplayTags::Get().Trigger_Projectile_Depleted,
			FGameplayTagContainer(),
			SpellContext
		);
	}
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
