// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityTask_MoveTowardTarget.h"

#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif

UAbilityTask_MoveTowardTarget::UAbilityTask_MoveTowardTarget()
{
	bTickingTask = false;
}

UAbilityTask_MoveTowardTarget* UAbilityTask_MoveTowardTarget::MoveTowardTarget(
	UGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	FVector InStartLocation,
	FVector InTargetLocation,
	float InMoveDistance,
	float InAcceptanceRadius,
	bool bInDrawDebug
)
{
	UAbilityTask_MoveTowardTarget* Task =
		NewAbilityTask<UAbilityTask_MoveTowardTarget>(OwningAbility, TaskInstanceName);

	Task->StartLocation = InStartLocation;
	Task->TargetLocation = InTargetLocation;
	Task->MoveDistance = InMoveDistance;
	Task->AcceptanceRadius = InAcceptanceRadius;
	Task->bDrawDebug = bInDrawDebug;

	return Task;
}

void UAbilityTask_MoveTowardTarget::Activate()
{
	Super::Activate();

	if (!Ability)
	{
		EndTask();
		return;
	}

	AActor* AvatarActor = GetAvatarActor();
	if (!AvatarActor)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnMoveFailed.Broadcast(FVector::ZeroVector);
		}

		EndTask();
		return;
	}

	// 이 Task는 AIController 기반 몬스터 이동용이다.
	// 클라이언트에서 실행될 경우 AIController가 없거나 의미가 없으므로 조용히 종료한다.
	if (!AvatarActor->HasAuthority())
	{
		EndTask();
		return;
	}

	APawn* AvatarPawn = Cast<APawn>(AvatarActor);
	if (!AvatarPawn)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnMoveFailed.Broadcast(FVector::ZeroVector);
		}

		EndTask();
		return;
	}

	AAIController* AIController = Cast<AAIController>(AvatarPawn->GetController());
	if (!AIController)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnMoveFailed.Broadcast(FVector::ZeroVector);
		}

		EndTask();
		return;
	}

	CachedPawn = AvatarPawn;
	CachedAIController = AIController;

	if (!ResolveMoveLocation(FinalMoveLocation))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnMoveFailed.Broadcast(FinalMoveLocation);
		}

		EndTask();
		return;
	}

	UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent();
	if (!PathFollowingComp)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnMoveFailed.Broadcast(FinalMoveLocation);
		}

		EndTask();
		return;
	}

	MoveFinishedDelegateHandle = PathFollowingComp->OnRequestFinished.AddUObject(
		this,
		&UAbilityTask_MoveTowardTarget::HandleMoveFinished
	);

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(FinalMoveLocation);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

	// 이 Task는 "돌진"이라기보다는 NavMesh 기반 접근 이동이다.
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true);
	MoveRequest.SetProjectGoalLocation(true);

	FNavPathSharedPtr NavPath;
	const FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest, &NavPath);

	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		CleanupMoveDelegate();

		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnMoveFailed.Broadcast(FinalMoveLocation);
		}

		EndTask();
		return;
	}

	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		CleanupMoveDelegate();

		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnRequestSuccess.Broadcast(FinalMoveLocation);
			OnMoveFinished.Broadcast(FinalMoveLocation);
		}

		EndTask();
		return;
	}

	ActiveMoveRequestID = MoveResult.MoveId;
	bMoveRequested = true;
	bStoppedMovement = false;

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnRequestSuccess.Broadcast(FinalMoveLocation);
	}
}

bool UAbilityTask_MoveTowardTarget::ResolveMoveLocation(FVector& OutMoveLocation) const
{
	const APawn* Pawn = CachedPawn.Get();
	if (!Pawn)
	{
		return false;
	}

	FVector ToTarget = TargetLocation - StartLocation;
	ToTarget.Z = 0.f;

	const FVector Direction = ToTarget.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector DesiredMoveLocation = StartLocation + Direction * MoveDistance;

	// 벽에 딱 붙지 않도록 약간 앞에서 멈춤.
	const float StopDistanceFromWall = 50.f;

	const FVector TraceStart = StartLocation;
	const FVector TraceEnd = DesiredMoveLocation;

	FHitResult Hit;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MoveTowardTarget_WallTrace), false);
	QueryParams.AddIgnoredActor(Pawn);

	const bool bHitWall = World->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHitWall && Hit.bBlockingHit)
	{
		const float HitDistance = FVector::Dist2D(StartLocation, Hit.ImpactPoint);
		const float MoveDistanceBeforeWall = FMath::Max(0.f, HitDistance - StopDistanceFromWall);

		OutMoveLocation = StartLocation + Direction * MoveDistanceBeforeWall;
		OutMoveLocation.Z = StartLocation.Z;

#if !UE_BUILD_SHIPPING
		if (bDrawDebug)
		{
			DrawDebugLine(World, TraceStart, TraceEnd, FColor::Yellow, false, 3.f, 0, 2.f);
			DrawDebugSphere(World, Hit.ImpactPoint, 15.f, 8, FColor::Red, false, 3.f);
			DrawDebugSphere(World, OutMoveLocation, 20.f, 8, FColor::Orange, false, 3.f);
		}
#endif

		return true;
	}

	OutMoveLocation = DesiredMoveLocation;
	OutMoveLocation.Z = StartLocation.Z;

#if !UE_BUILD_SHIPPING
	if (bDrawDebug)
	{
		DrawDebugLine(World, TraceStart, TraceEnd, FColor::Yellow, false, 3.f, 0, 2.f);
		DrawDebugSphere(World, OutMoveLocation, 20.f, 8, FColor::Green, false, 3.f);
	}
#endif

	return true;
}

void UAbilityTask_MoveTowardTarget::HandleMoveFinished(
	FAIRequestID RequestID,
	const FPathFollowingResult& Result
)
{
	if (!bMoveRequested)
	{
		return;
	}

	if (ActiveMoveRequestID.IsValid() && RequestID != ActiveMoveRequestID)
	{
		return;
	}

	bMoveRequested = false;

	CleanupMoveDelegate();

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (Result.IsSuccess())
		{
			OnMoveFinished.Broadcast(FinalMoveLocation);
		}
		else
		{
			OnMoveFailed.Broadcast(FinalMoveLocation);
		}
	}

	EndTask();
}

void UAbilityTask_MoveTowardTarget::ExternalCancel()
{
	StopMovement();
	Super::ExternalCancel();
}

void UAbilityTask_MoveTowardTarget::StopMovement()
{
	if (bStoppedMovement)
	{
		return;
	}

	bStoppedMovement = true;

	const bool bHadActiveMoveRequest = bMoveRequested;
	bMoveRequested = false;

	CleanupMoveDelegate();

	if (!bHadActiveMoveRequest)
	{
		return;
	}

	if (AAIController* AIController = CachedAIController.Get())
	{
		AIController->StopMovement();
	}
}

void UAbilityTask_MoveTowardTarget::CleanupMoveDelegate()
{
	if (!MoveFinishedDelegateHandle.IsValid())
	{
		return;
	}

	if (AAIController* AIController = CachedAIController.Get())
	{
		if (UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent())
		{
			PathFollowingComp->OnRequestFinished.Remove(MoveFinishedDelegateHandle);
		}
	}

	MoveFinishedDelegateHandle.Reset();
}

void UAbilityTask_MoveTowardTarget::OnDestroy(bool bInOwnerFinished)
{
	// 핵심:
	// 정상 완료 후 EndTask -> OnDestroy로 들어온 경우에는 StopMovement를 호출하지 않는다.
	// 그렇지 않으면 OnMoveFinished에서 다음 이동/공격을 시작했을 때 그 이동까지 끊을 수 있다.
	if (bMoveRequested)
	{
		StopMovement();
	}
	else
	{
		CleanupMoveDelegate();
	}

	Super::OnDestroy(bInOwnerFinished);
}