#include "AbilityTask_ChargeDirectly.h"

#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UAbilityTask_ChargeDirectly::UAbilityTask_ChargeDirectly(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UAbilityTask_ChargeDirectly* UAbilityTask_ChargeDirectly::ChargeTowardLocation(
    UGameplayAbility* OwningAbility,
    FVector TargetLocation,
    float InDuration,
    float InChargeDistance,
    float InAcceptanceRadius,
    bool bInStopExistingMovement,
    bool bInStopOnEnd
)
{
    UAbilityTask_ChargeDirectly* Task =
        NewAbilityTask<UAbilityTask_ChargeDirectly>(OwningAbility);

    Task->InputMode = EChargeInputMode::TowardLocation;
    Task->InputTargetLocation = TargetLocation;
    Task->Duration = InDuration;
    Task->ChargeDistance = InChargeDistance;
    Task->AcceptanceRadius = FMath::Max(1.0f, InAcceptanceRadius);
    Task->bStopExistingMovement = bInStopExistingMovement;
    Task->bStopOnEnd = bInStopOnEnd;

    return Task;
}

UAbilityTask_ChargeDirectly* UAbilityTask_ChargeDirectly::ChargeInDirection(
    UGameplayAbility* OwningAbility,
    FVector Direction,
    float InDuration,
    float InChargeDistance,
    float InAcceptanceRadius,
    bool bInStopExistingMovement,
    bool bInStopOnEnd
)
{
    UAbilityTask_ChargeDirectly* Task =
        NewAbilityTask<UAbilityTask_ChargeDirectly>(OwningAbility);

    Task->InputMode = EChargeInputMode::Direction;
    Task->InputDirection = Direction;
    Task->Duration = InDuration;
    Task->ChargeDistance = InChargeDistance;
    Task->AcceptanceRadius = FMath::Max(1.0f, InAcceptanceRadius);
    Task->bStopExistingMovement = bInStopExistingMovement;
    Task->bStopOnEnd = bInStopOnEnd;

    return Task;
}

void UAbilityTask_ChargeDirectly::Activate()
{
    Super::Activate();

    if (Duration <= 0.0f)
    {
        FinishFailed();
        return;
    }

    if (!CacheAvatarAndController())
    {
        FinishFailed();
        return;
    }

    FVector GoalLocation;
    if (!BuildChargeGoal(GoalLocation))
    {
        FinishFailed();
        return;
    }

    StartAIMove(GoalLocation);
}

bool UAbilityTask_ChargeDirectly::CacheAvatarAndController()
{
    if (!Ability)
    {
        return false;
    }

    const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
    if (!ActorInfo)
    {
        return false;
    }

    APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
    if (!Pawn)
    {
        return false;
    }

    AAIController* AIController = Cast<AAIController>(Pawn->GetController());
    if (!AIController)
    {
        return false;
    }

    CachedPawn = Pawn;
    CachedAIController = AIController;

    if (ACharacter* Character = Cast<ACharacter>(Pawn))
    {
        CachedCharacterMovement = Character->GetCharacterMovement();
    }

    return true;
}

bool UAbilityTask_ChargeDirectly::BuildChargeGoal(FVector& OutGoalLocation) const
{
    const APawn* Pawn = CachedPawn.Get();
    if (!Pawn)
    {
        return false;
    }

    const FVector StartLocation = Pawn->GetActorLocation();

    FVector ChargeDirection = FVector::ZeroVector;

    if (InputMode == EChargeInputMode::TowardLocation)
    {
        ChargeDirection = InputTargetLocation - StartLocation;
    }
    else
    {
        ChargeDirection = InputDirection;
    }

    ChargeDirection = ChargeDirection.GetSafeNormal();

    if (ChargeDirection.IsNearlyZero())
    {
        return false;
    }

    const float FinalDistance = ResolveChargeDistance();
    if (FinalDistance <= 0.0f)
    {
        return false;
    }

    OutGoalLocation = StartLocation + ChargeDirection * FinalDistance;
    return true;
}

float UAbilityTask_ChargeDirectly::ResolveChargeDistance() const
{
    if (ChargeDistance > 0.0f)
    {
        return ChargeDistance;
    }

    if (const UCharacterMovementComponent* MoveComp = CachedCharacterMovement.Get())
    {
        return MoveComp->GetMaxSpeed() * Duration;
    }

    return 0.0f;
}

void UAbilityTask_ChargeDirectly::StartAIMove(const FVector& GoalLocation)
{
    AAIController* AIController = CachedAIController.Get();
    if (!AIController)
    {
        FinishFailed();
        return;
    }

    UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent();
    if (!PathFollowingComp)
    {
        FinishFailed();
        return;
    }

    if (bStopExistingMovement)
    {
        AIController->StopMovement();
    }

    MoveFinishedHandle = PathFollowingComp->OnRequestFinished.AddUObject(
        this,
        &UAbilityTask_ChargeDirectly::HandleMoveFinished
    );

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(GoalLocation);
    MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

    // 핵심: NavMesh 경로 탐색을 쓰지 않고 직선 이동.
    MoveRequest.SetUsePathfinding(false);

    // 핵심: 공중 목적지를 NavMesh 위로 투영하지 않음.
    MoveRequest.SetProjectGoalLocation(false);

    // 공중 돌진 목적지가 NavMesh 위에 있을 필요 없음.
    MoveRequest.SetRequireNavigableEndLocation(false);

    MoveRequest.SetAllowPartialPath(false);
    MoveRequest.SetCanStrafe(true);

    // 공중 돌진에서는 직접 설정한 AcceptanceRadius만 쓰는 편이 예측하기 쉬움.
    MoveRequest.SetReachTestIncludesAgentRadius(false);
    MoveRequest.SetReachTestIncludesGoalRadius(false);

    const FPathFollowingRequestResult MoveResult = AIController->MoveTo(MoveRequest);

    if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        FinishCompleted();
        return;
    }

    if (MoveResult.Code == EPathFollowingRequestResult::Failed)
    {
        FinishFailed();
        return;
    }

    ActiveMoveRequestID = MoveResult.MoveId;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DurationTimerHandle,
            this,
            &UAbilityTask_ChargeDirectly::HandleDurationFinished,
            Duration,
            false
        );
    }

    SetWaitingOnAvatar();
}

void UAbilityTask_ChargeDirectly::HandleMoveFinished(
    FAIRequestID RequestID,
    const FPathFollowingResult& Result
)
{
    if (bFinished)
    {
        return;
    }

    if (ActiveMoveRequestID.IsValid() && RequestID != ActiveMoveRequestID)
    {
        return;
    }

    switch (Result.Code)
    {
    case EPathFollowingResult::Success:
        FinishCompleted();
        break;

    case EPathFollowingResult::Blocked:
        FinishBlocked();
        break;

    case EPathFollowingResult::Aborted:
    case EPathFollowingResult::OffPath:
    case EPathFollowingResult::Invalid:
    default:
        FinishFailed();
        break;
    }
}

void UAbilityTask_ChargeDirectly::HandleDurationFinished()
{
    if (bFinished)
    {
        return;
    }

    FinishDurationFinished();
}

void UAbilityTask_ChargeDirectly::FinishCompleted()
{
    FinishWithDelegate(OnCompleted);
}

void UAbilityTask_ChargeDirectly::FinishBlocked()
{
    FinishWithDelegate(OnBlocked);
}

void UAbilityTask_ChargeDirectly::FinishDurationFinished()
{
    FinishWithDelegate(OnDurationFinished);
}

void UAbilityTask_ChargeDirectly::FinishFailed()
{
    FinishWithDelegate(OnFailed);
}

void UAbilityTask_ChargeDirectly::FinishWithDelegate(FChargeDirectlyDelegate& Delegate)
{
    if (bFinished)
    {
        return;
    }

    bFinished = true;

    Cleanup();

    if (bStopOnEnd)
    {
        if (AAIController* AIController = CachedAIController.Get())
        {
            AIController->StopMovement();
        }
    }

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        Delegate.Broadcast();
    }

    EndTask();
}

void UAbilityTask_ChargeDirectly::Cleanup()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DurationTimerHandle);
    }

    if (AAIController* AIController = CachedAIController.Get())
    {
        if (UPathFollowingComponent* PathFollowingComp = AIController->GetPathFollowingComponent())
        {
            if (MoveFinishedHandle.IsValid())
            {
                PathFollowingComp->OnRequestFinished.Remove(MoveFinishedHandle);
                MoveFinishedHandle.Reset();
            }
        }
    }
}

void UAbilityTask_ChargeDirectly::OnDestroy(bool bInOwnerFinished)
{
    Cleanup();

    if (!bFinished && bStopOnEnd)
    {
        if (AAIController* AIController = CachedAIController.Get())
        {
            AIController->StopMovement();
        }
    }

    Super::OnDestroy(bInOwnerFinished);
}