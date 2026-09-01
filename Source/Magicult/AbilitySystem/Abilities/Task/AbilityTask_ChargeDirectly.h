#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AITypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "AbilityTask_ChargeDirectly.generated.h"

class AAIController;
class APawn;
class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChargeDirectlyDelegate);

/**
 * GA 안에서 사용하는 직선 돌진 AbilityTask.
 * 시작 시점 방향만 계산해 유지하고, NavMesh Pathfinding/투영을 꺼서
 * 공중 목표로도 직선 돌진이 가능하게 한다.
 */
UCLASS()
class MAGICULT_API UAbilityTask_ChargeDirectly : public UAbilityTask
{
    GENERATED_BODY()

public:
    UAbilityTask_ChargeDirectly(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(BlueprintAssignable)
    FChargeDirectlyDelegate OnCompleted;

    UPROPERTY(BlueprintAssignable)
    FChargeDirectlyDelegate OnBlocked;

    UPROPERTY(BlueprintAssignable)
    FChargeDirectlyDelegate OnDurationFinished;

    UPROPERTY(BlueprintAssignable)
    FChargeDirectlyDelegate OnFailed;

    /**
     * TargetLocation 방향으로 돌진.
     * TargetLocation을 계속 추적하지 않고, 시작 시점 방향만 사용한다.
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Ability|Tasks",
        meta = (
            DisplayName = "Charge Toward Location",
            HidePin = "OwningAbility",
            DefaultToSelf = "OwningAbility",
            BlueprintInternalUseOnly = "true"
            )
    )
    static UAbilityTask_ChargeDirectly* ChargeTowardLocation(
        UGameplayAbility* OwningAbility,
        FVector TargetLocation,
        float Duration = 0.7f,
        float ChargeDistance = 0.0f,
        float AcceptanceRadius = 80.0f,
        bool bStopExistingMovement = true,
        bool bStopOnEnd = true
    );

    /**
     * 지정 방향으로 돌진.
     */
    UFUNCTION(
        BlueprintCallable,
        Category = "Ability|Tasks",
        meta = (
            DisplayName = "Charge In Direction",
            HidePin = "OwningAbility",
            DefaultToSelf = "OwningAbility",
            BlueprintInternalUseOnly = "true"
            )
    )
    static UAbilityTask_ChargeDirectly* ChargeInDirection(
        UGameplayAbility* OwningAbility,
        FVector Direction,
        float Duration = 0.7f,
        float ChargeDistance = 0.0f,
        float AcceptanceRadius = 80.0f,
        bool bStopExistingMovement = true,
        bool bStopOnEnd = true
    );

    virtual void Activate() override;
    virtual void OnDestroy(bool bInOwnerFinished) override;

private:
    enum class EChargeInputMode : uint8
    {
        TowardLocation,
        Direction
    };

private:
    EChargeInputMode InputMode = EChargeInputMode::TowardLocation;

    FVector InputTargetLocation = FVector::ZeroVector;
    FVector InputDirection = FVector::ZeroVector;

    float Duration = 0.7f;

    /**
     * 0 이하이면 현재 CharacterMovement의 GetMaxSpeed() * Duration으로 거리 계산.
     * Flying 상태라면 MaxFlySpeed 기반으로 계산된다.
     */
    float ChargeDistance = 0.0f;

    float AcceptanceRadius = 80.0f;

    bool bStopExistingMovement = true;
    bool bStopOnEnd = true;

    TWeakObjectPtr<APawn> CachedPawn;
    TWeakObjectPtr<AAIController> CachedAIController;
    TWeakObjectPtr<UCharacterMovementComponent> CachedCharacterMovement;

    FAIRequestID ActiveMoveRequestID;
    FDelegateHandle MoveFinishedHandle;
    FTimerHandle DurationTimerHandle;

    bool bFinished = false;

private:
    bool CacheAvatarAndController();
    bool BuildChargeGoal(FVector& OutGoalLocation) const;
    float ResolveChargeDistance() const;

    void StartAIMove(const FVector& GoalLocation);

    void HandleMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);
    void HandleDurationFinished();

    void FinishCompleted();
    void FinishBlocked();
    void FinishDurationFinished();
    void FinishFailed();

    void FinishWithDelegate(FChargeDirectlyDelegate& Delegate);
    void Cleanup();
};