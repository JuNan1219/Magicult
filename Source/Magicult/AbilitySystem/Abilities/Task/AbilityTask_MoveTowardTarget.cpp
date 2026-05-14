// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_MoveTowardTarget.h"
#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"

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
	float InAcceptanceRadius
)
{
	UAbilityTask_MoveTowardTarget* Task =
		NewAbilityTask<UAbilityTask_MoveTowardTarget>(OwningAbility, TaskInstanceName);

	Task->StartLocation = InStartLocation;
	Task->TargetLocation = InTargetLocation;
	Task->MoveDistance = InMoveDistance;
	Task->AcceptanceRadius = InAcceptanceRadius;

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

	APawn* AvatarPawn = Cast<APawn>(GetAvatarActor());
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
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true);
	MoveRequest.SetProjectGoalLocation(true);

	FNavPathSharedPtr NavPath;
	FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest, &NavPath);

	if (MoveResult.Code == EPathFollowingRequestResult::Failed)
	{
		if (MoveFinishedDelegateHandle.IsValid())
		{
			PathFollowingComp->OnRequestFinished.Remove(MoveFinishedDelegateHandle);
			MoveFinishedDelegateHandle.Reset();
		}

		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnMoveFailed.Broadcast(FinalMoveLocation);
		}

		EndTask();
		return;
	}

	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		if (MoveFinishedDelegateHandle.IsValid())
		{
			PathFollowingComp->OnRequestFinished.Remove(MoveFinishedDelegateHandle);
			MoveFinishedDelegateHandle.Reset();
		}

		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnRequestSuccess.Broadcast(FinalMoveLocation);
			// 별도 OnMoveFinished / OnReached 같은 delegate가 있으면 그걸 호출하는 게 더 적절
		}

		EndTask();
		return;
	}

	ActiveMoveRequestID = MoveResult.MoveId;
	bMoveRequested = true;

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnRequestSuccess.Broadcast(FinalMoveLocation);
	}
}

bool UAbilityTask_MoveTowardTarget::ResolveMoveLocation(FVector& OutMoveLocation) const
{
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

	// 레이는 바닥이 아니라 몬스터 몸통 높이에서 쏘는 게 벽 감지가 잘 됨.
	const float TraceHeight = 80.f;

	// 벽에 딱 붙지 않도록 약간 앞에서 멈춤.
	const float StopDistanceFromWall = 50.f;

	const FVector TraceStart = StartLocation + FVector(0.f, 0.f, TraceHeight);
	const FVector TraceEnd = DesiredMoveLocation + FVector(0.f, 0.f, TraceHeight);

	FHitResult Hit;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MoveTowardTarget_WallTrace), false);

	if (const AActor* AvatarActor = CachedPawn.Get())
	{
		QueryParams.AddIgnoredActor(AvatarActor);
	}

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

		return true;
	}

	OutMoveLocation = DesiredMoveLocation;
	OutMoveLocation.Z = StartLocation.Z;

	return true;
}

void UAbilityTask_MoveTowardTarget::HandleMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (!bMoveRequested || RequestID != ActiveMoveRequestID)
	{
		return;
	}

	bMoveRequested = false;

	if (AAIController* AIController = CachedAIController.Get())
	{
		if (UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent())
		{
			if (MoveFinishedDelegateHandle.IsValid())
			{
				PathFollowingComp->OnRequestFinished.Remove(MoveFinishedDelegateHandle);
				MoveFinishedDelegateHandle.Reset();
			}
		}
	}

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

	if (AAIController* AIController = CachedAIController.Get())
	{
		if (UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent())
		{
			if (MoveFinishedDelegateHandle.IsValid())
			{
				PathFollowingComp->OnRequestFinished.Remove(MoveFinishedDelegateHandle);
				MoveFinishedDelegateHandle.Reset();
			}
		}

		AIController->StopMovement();
	}
}

void UAbilityTask_MoveTowardTarget::OnDestroy(bool bInOwnerFinished)
{
	StopMovement();
	Super::OnDestroy(bInOwnerFinished);
}