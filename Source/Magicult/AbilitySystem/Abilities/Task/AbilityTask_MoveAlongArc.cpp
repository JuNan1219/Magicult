// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_MoveAlongArc.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Actor.h"
#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#include "Magicult/Helper/MagicultDebug.h"
#endif

UAbilityTask_MoveAlongArc* UAbilityTask_MoveAlongArc::MoveAlongArc(UGameplayAbility* OwningAbility, FVector InStartLocation, FVector InEndLocation, float InDuration, float InArcHeight, float EndPhaseThreshold, bool bInDrawDebug)
{
    UAbilityTask_MoveAlongArc* Task = NewAbilityTask<UAbilityTask_MoveAlongArc>(OwningAbility);
    Task->StartLocation = InStartLocation;
    Task->EndLocation = InEndLocation;
    Task->Duration = FMath::Max(0.01f, InDuration);
    Task->ArcHeight = InArcHeight;
    Task->EndPhaseThreshold = EndPhaseThreshold;
    Task->bDrawDebug = bInDrawDebug;
    return Task;
}

void UAbilityTask_MoveAlongArc::Activate()
{
    Super::Activate();

    const FVector MidPoint = (StartLocation + EndLocation) * 0.5f;
    ApexLocation = MidPoint + FVector(0.f, 0.f, ArcHeight);

    ElapsedTime = 0.0f;
    bTickingTask = true;

#if !UE_BUILD_SHIPPING
    if (bDrawDebug && IsMagicultDrawDebugEnabled())
    {
        if (UWorld* World = GetWorld())
        {
            const float DrawDuration = Duration + 1.f;
            const int32 NumSegments = 24;
            FVector PrevPoint = StartLocation;
            for (int32 i = 1; i <= NumSegments; i++)
            {
                const float A = static_cast<float>(i) / static_cast<float>(NumSegments);
                const FVector NextPoint = EvaluatePosition(A);
                DrawDebugLine(World, PrevPoint, NextPoint, FColor::Cyan, false, DrawDuration, 0, 2.f);
                PrevPoint = NextPoint;
            }
            DrawDebugSphere(World, StartLocation, 15.f, 8, FColor::Green, false, DrawDuration);
            DrawDebugSphere(World, EndLocation, 15.f, 8, FColor::Red, false, DrawDuration);
            DrawDebugSphere(World, ApexLocation, 10.f, 8, FColor::Yellow, false, DrawDuration);
        }
    }
#endif
}

void UAbilityTask_MoveAlongArc::TickTask(float DeltaTime)
{
    Super::TickTask(DeltaTime);

    AActor* AvatarActor = GetAvatarActor();
    if (!AvatarActor)
    {
        EndTask();
        return;
    }

    ElapsedTime += DeltaTime;
    const float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);

    const FVector NewLocation = EvaluatePosition(Alpha);
    AvatarActor->SetActorLocation(NewLocation, true);

    if (Alpha >= EndPhaseThreshold && !bIsEndPhase)
    {
        bIsEndPhase = true;
        OnEndPhase.Broadcast();
    }

    if (Alpha >= 1.0f)
    {
        EndTask();
    }
}

void UAbilityTask_MoveAlongArc::OnDestroy(bool bInOwnerFinished)
{
    bTickingTask = false;
    Super::OnDestroy(bInOwnerFinished);
}

FVector UAbilityTask_MoveAlongArc::EvaluatePosition(float Alpha) const
{
    const float OneMinusAlpha = 1.0f - Alpha;

    // 2차 베지어 곡선
    return (OneMinusAlpha * OneMinusAlpha) * StartLocation + 
        (2.0f * OneMinusAlpha * Alpha) * ApexLocation + 
        (Alpha * Alpha) * EndLocation;
}