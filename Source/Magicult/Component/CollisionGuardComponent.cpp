#include "CollisionGuardComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UCollisionGuardComponent::UCollisionGuardComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCollisionGuardComponent::BeginPlay()
{
	Super::BeginPlay();
	BindCollisionComponent();
	SetComponentTickEnabled(bUseComponentTickForRehit);
}

void UCollisionGuardComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindCollisionComponent();
	ResetGuard();
	Super::EndPlay(EndPlayReason);
}

void UCollisionGuardComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bUseComponentTickForRehit)
	{
		ProcessOverlapRehit(DeltaTime);
	}
}

void UCollisionGuardComponent::SetCollisionComponent(UPrimitiveComponent* InCollisionComponent)
{
	if (CollisionComponent == InCollisionComponent)
	{
		return;
	}
	UnbindCollisionComponent();
	CollisionComponent = InCollisionComponent;
	BindCollisionComponent();
}

void UCollisionGuardComponent::BindCollisionComponent()
{
	if (!CollisionComponent)
	{
		return;
	}

	if (bHandleOverlap)
	{
		CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleBeginOverlap);
		CollisionComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::HandleEndOverlap);
	}

	if (bHandleHit)
	{
		CollisionComponent->OnComponentHit.AddUniqueDynamic(this, &ThisClass::HandleHit);
	}
}

void UCollisionGuardComponent::UnbindCollisionComponent()
{
	if (!CollisionComponent)
	{
		return;
	}

	CollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::HandleBeginOverlap);
	CollisionComponent->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::HandleEndOverlap);
	CollisionComponent->OnComponentHit.RemoveDynamic(this, &ThisClass::HandleHit);
}

void UCollisionGuardComponent::ResetGuard()
{
	HitActorsThisLife.Reset();
	NextAllowedHitTime.Reset();
	ActiveOverlaps.Reset();
}

void UCollisionGuardComponent::ResetActor(AActor* Actor)
{
	if (!Actor) return;
	const TWeakObjectPtr<AActor> Key(Actor);
	HitActorsThisLife.Remove(Key);
	NextAllowedHitTime.Remove(Key);
	ActiveOverlaps.Remove(Key);
}

bool UCollisionGuardComponent::IsAuthorityAllowed() const
{
	if (!bRequireAuthority) return true;
	const AActor* Owner = GetOwner();
	return Owner && Owner->HasAuthority();
}

bool UCollisionGuardComponent::IsValidImpact(AActor* OtherActor) const
{
	if (!OtherActor) return false;
	const AActor* Owner = GetOwner();
	if (bIgnoreOwner && (OtherActor == Owner || OtherActor == Owner->GetInstigator())) return false;
	if (IgnoredActors.Contains(OtherActor)) return false;
	return true;
}

bool UCollisionGuardComponent::CanPublish(AActor* OtherActor)
{
	if (!IsAuthorityAllowed()) return false;
	if (!IsValidImpact(OtherActor)) return false;

	const TWeakObjectPtr<AActor> Key(OtherActor);

	if (!bCanHitSameTargetAgain)
	{
		if (HitActorsThisLife.Contains(Key)) return false;
		HitActorsThisLife.Add(Key);
		return true;
	}

	UWorld* World = GetWorld();
	if (!World) return false;

	const float Now = World->GetTimeSeconds();
	if (const float* NextTime = NextAllowedHitTime.Find(Key))
	{
		if (Now < *NextTime) return false;
	}
	NextAllowedHitTime.Add(Key, Now + RehitCooldown);
	return true;
}

void UCollisionGuardComponent::Broadcast(const FGuardedCollisionEvent& Event)
{
	OnGuardedCollision.Broadcast(Event);
}

void UCollisionGuardComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	FGuardedCollisionEvent Event;
	Event.CollisionType    = EGuardedCollisionType::Overlap;
	Event.SourceComponent  = OverlappedComponent;
	Event.OtherActor       = OtherActor;
	Event.OtherComponent   = OtherComp;
	Event.HitResult        = SweepResult;
	Event.bIsRehit         = false;
	Event.bFromSweep       = bFromSweep;

	// Overlap은 지속 여부 추적이 필요하므로 ActiveOverlaps에 등록
	ActiveOverlaps.Add(TWeakObjectPtr<AActor>(OtherActor), Event);

	if (!CanPublish(OtherActor)) return;
	Broadcast(Event);
}

void UCollisionGuardComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	ActiveOverlaps.Remove(TWeakObjectPtr<AActor>(OtherActor));
}

void UCollisionGuardComponent::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor) return;
	if (!CanPublish(OtherActor)) return;

	FGuardedCollisionEvent Event;
	Event.CollisionType   = EGuardedCollisionType::Hit;
	Event.SourceComponent = HitComponent;
	Event.OtherActor      = OtherActor;
	Event.OtherComponent  = OtherComp;
	Event.HitResult       = Hit;
	Event.bIsRehit        = false;

	Broadcast(Event);
}

void UCollisionGuardComponent::ProcessOverlapRehit(float DeltaTime)
{
	if (!bCanHitSameTargetAgain) return;
	if (!IsAuthorityAllowed()) return;

	CleanupInvalidReferences();

	TArray<FGuardedCollisionEvent> ToFire;
	for (const TPair<TWeakObjectPtr<AActor>, FGuardedCollisionEvent>& Pair : ActiveOverlaps)
	{
		AActor* OtherActor = Pair.Key.Get();
		if (!OtherActor) continue;
		if (!CanPublish(OtherActor)) continue;

		FGuardedCollisionEvent RehitEvent = Pair.Value;
		RehitEvent.OtherActor  = OtherActor;
		RehitEvent.bIsRehit    = true;
		RehitEvent.HitResult   = FHitResult();
		ToFire.Add(RehitEvent);
	}

	for (const FGuardedCollisionEvent& Event : ToFire)
	{
		Broadcast(Event);
	}

	CleanupInvalidReferences();
}

void UCollisionGuardComponent::CleanupInvalidReferences()
{
	for (auto It = HitActorsThisLife.CreateIterator(); It; ++It)
	{
		if (!It->IsValid()) It.RemoveCurrent();
	}
	for (auto It = NextAllowedHitTime.CreateIterator(); It; ++It)
	{
		if (!It->Key.IsValid()) It.RemoveCurrent();
	}
	for (auto It = ActiveOverlaps.CreateIterator(); It; ++It)
	{
		if (!It->Key.IsValid()) It.RemoveCurrent();
	}
}
