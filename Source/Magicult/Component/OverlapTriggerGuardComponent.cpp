#include "OverlapTriggerGuardComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

UOverlapTriggerGuardComponent::UOverlapTriggerGuardComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UOverlapTriggerGuardComponent::BeginPlay()
{
	Super::BeginPlay();

	BindCollisionComponent();

	SetComponentTickEnabled(bUseComponentTickForRehit);
}

void UOverlapTriggerGuardComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindCollisionComponent();
	ResetGuard();

	Super::EndPlay(EndPlayReason);
}

void UOverlapTriggerGuardComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bUseComponentTickForRehit)
	{
		ProcessOverlapRehit(DeltaTime);
	}
}

void UOverlapTriggerGuardComponent::SetCollisionComponent(UPrimitiveComponent* InCollisionComponent)
{
	if (CollisionComponent == InCollisionComponent)
	{
		return;
	}

	UnbindCollisionComponent();

	CollisionComponent = InCollisionComponent;

	BindCollisionComponent();
}

void UOverlapTriggerGuardComponent::BindCollisionComponent()
{
	if (!CollisionComponent)
	{
		return;
	}

	CollisionComponent->OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&ThisClass::HandleBeginOverlap
	);

	CollisionComponent->OnComponentEndOverlap.AddUniqueDynamic(
		this,
		&ThisClass::HandleEndOverlap
	);
}

void UOverlapTriggerGuardComponent::UnbindCollisionComponent()
{
	if (!CollisionComponent)
	{
		return;
	}

	CollisionComponent->OnComponentBeginOverlap.RemoveDynamic(
		this,
		&ThisClass::HandleBeginOverlap
	);

	CollisionComponent->OnComponentEndOverlap.RemoveDynamic(
		this,
		&ThisClass::HandleEndOverlap
	);
}

void UOverlapTriggerGuardComponent::ResetGuard()
{
	HitActorsThisLife.Reset();
	NextAllowedHitTime.Reset();
	ActiveOverlaps.Reset();
}

void UOverlapTriggerGuardComponent::ResetActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	const TWeakObjectPtr<AActor> Key(Actor);

	HitActorsThisLife.Remove(Key);
	NextAllowedHitTime.Remove(Key);
	ActiveOverlaps.Remove(Key);
}

bool UOverlapTriggerGuardComponent::IsAuthorityAllowed() const
{
	if (!bRequireAuthority)
	{
		return true;
	}

	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

bool UOverlapTriggerGuardComponent::IsValidImpact(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	const AActor* OwnerActor = GetOwner();

	if (bIgnoreOwner && (OtherActor == OwnerActor || OtherActor ==OwnerActor->GetInstigator()))
	{
		return false;
	}

	if (IgnoredActors.Contains(OtherActor))
	{
		return false;
	}

	return true;
}

bool UOverlapTriggerGuardComponent::CanPublishOverlap(AActor* OtherActor)
{
	if (!IsAuthorityAllowed())
	{
		return false;
	}

	if (!IsValidImpact(OtherActor))
	{
		return false;
	}

	const TWeakObjectPtr<AActor> TargetKey(OtherActor);

	if (!bCanHitSameTargetAgain)
	{
		if (HitActorsThisLife.Contains(TargetKey))
		{
			return false;
		}

		HitActorsThisLife.Add(TargetKey);
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float Now = World->GetTimeSeconds();

	if (const float* NextTime = NextAllowedHitTime.Find(TargetKey))
	{
		if (Now < *NextTime)
		{
			return false;
		}
	}

	NextAllowedHitTime.Add(TargetKey, Now + RehitCooldown);
	return true;
}

void UOverlapTriggerGuardComponent::RegisterActiveOverlap(const FGuardedOverlapEvent& Event)
{
	if (!Event.OtherActor)
	{
		return;
	}

	const TWeakObjectPtr<AActor> TargetKey(Event.OtherActor);
	ActiveOverlaps.Add(TargetKey, Event);
}

void UOverlapTriggerGuardComponent::BroadcastOverlap(const FGuardedOverlapEvent& Event)
{
	OnGuardedOverlap.Broadcast(Event);
}

void UOverlapTriggerGuardComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	FGuardedOverlapEvent Event;
	Event.SourceComponent = OverlappedComponent;
	Event.OtherActor = OtherActor;
	Event.OtherComponent = OtherComp;
	Event.OtherBodyIndex = OtherBodyIndex;
	Event.bFromSweep = bFromSweep;
	Event.SweepResult = SweepResult;
	Event.bIsRehit = false;

	RegisterActiveOverlap(Event);

	if (!CanPublishOverlap(OtherActor))
	{
		return;
	}

	BroadcastOverlap(Event);
}

void UOverlapTriggerGuardComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	const TWeakObjectPtr<AActor> TargetKey(OtherActor);
	ActiveOverlaps.Remove(TargetKey);
}

void UOverlapTriggerGuardComponent::ProcessOverlapRehit(float DeltaTime)
{
	if (!bCanHitSameTargetAgain)
	{
		return;
	}

	if (!IsAuthorityAllowed())
	{
		return;
	}

	CleanupInvalidReferences();

	TArray<FGuardedOverlapEvent> EventsToBroadcast;

	for (const TPair<TWeakObjectPtr<AActor>, FGuardedOverlapEvent>& Pair : ActiveOverlaps)
	{
		AActor* OtherActor = Pair.Key.Get();

		if (!OtherActor)
		{
			continue;
		}

		if (!CanPublishOverlap(OtherActor))
		{
			continue;
		}

		FGuardedOverlapEvent RehitEvent = Pair.Value;
		RehitEvent.OtherActor = OtherActor;
		RehitEvent.bFromSweep = false;
		RehitEvent.bIsRehit = true;
		RehitEvent.SweepResult = FHitResult();

		EventsToBroadcast.Add(RehitEvent);
	}

	for (const FGuardedOverlapEvent& Event : EventsToBroadcast)
	{
		BroadcastOverlap(Event);
	}

	CleanupInvalidReferences();
}

void UOverlapTriggerGuardComponent::CleanupInvalidReferences()
{
	TArray<TWeakObjectPtr<AActor>> InvalidHitActors;
	for (const TWeakObjectPtr<AActor>& WeakActor : HitActorsThisLife)
	{
		if (!WeakActor.IsValid())
		{
			InvalidHitActors.Add(WeakActor);
		}
	}

	for (const TWeakObjectPtr<AActor>& WeakActor : InvalidHitActors)
	{
		HitActorsThisLife.Remove(WeakActor);
	}

	TArray<TWeakObjectPtr<AActor>> InvalidCooldownActors;
	for (const TPair<TWeakObjectPtr<AActor>, float>& Pair : NextAllowedHitTime)
	{
		if (!Pair.Key.IsValid())
		{
			InvalidCooldownActors.Add(Pair.Key);
		}
	}

	for (const TWeakObjectPtr<AActor>& WeakActor : InvalidCooldownActors)
	{
		NextAllowedHitTime.Remove(WeakActor);
	}

	TArray<TWeakObjectPtr<AActor>> InvalidActiveActors;
	for (const TPair<TWeakObjectPtr<AActor>, FGuardedOverlapEvent>& Pair : ActiveOverlaps)
	{
		if (!Pair.Key.IsValid())
		{
			InvalidActiveActors.Add(Pair.Key);
		}
	}

	for (const TWeakObjectPtr<AActor>& WeakActor : InvalidActiveActors)
	{
		ActiveOverlaps.Remove(WeakActor);
	}
}
