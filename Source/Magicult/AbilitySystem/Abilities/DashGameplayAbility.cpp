// Fill out your copyright notice in the Description page of Project Settings.

#include "DashGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "Magicult/MagicultGameplayTags.h"

UDashGameplayAbility::UDashGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bRetriggerInstancedAbility = false;
	ActivationBlockedTags.AddTag(FMagicultGameplayTags::Get().Debuff_Stunned);
}

void UDashGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bIsEndingAbility = false;

	ACharacter* Character = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector Direction = ResolveDashDirection(TriggerEventData);
	if (Direction.IsNearlyZero())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 몽타주가 올바른 방향을 향하도록 캐릭터 회전
	Character->SetActorRotation(Direction.Rotation());

	const FVector TargetLocation = Character->GetActorLocation() + Direction * DashDistance;

	DashTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
		this,
		FName(TEXT("DashMoveToForce")),
		TargetLocation,
		DashDuration,
		false,
		EMovementMode::MOVE_Walking,
		true,
		nullptr,
		ERootMotionFinishVelocityMode::ClampVelocity,
		FVector::ZeroVector,
		FinishVelocityClamp);

	if (!DashTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 대시 종료 기준은 RootMotion DashTask 하나로 통일.
	// MoveToForce 완료 델리게이트는 OnTimedOut / OnTimedOutAndDestinationReached 둘뿐(상호 배타).
	DashTask->OnTimedOut.AddDynamic(this, &ThisClass::OnDashFinished);
	DashTask->OnTimedOutAndDestinationReached.AddDynamic(this, &ThisClass::OnDashFinished);
	DashTask->ReadyForActivation();

	if (DashMontage)
	{
		PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("DashMontage"),
			DashMontage,
			1.f,
			NAME_None,
			true,
			1.f);

		if (PlayMontageTask)
		{
			// Completed에서는 EndAbility 하지 않음.
			// DashTask와 MontageTask가 둘 다 EndAbility를 부르면 중복 종료 로그가 발생할 수 있음.
			PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
			PlayMontageTask->ReadyForActivation();
		}
	}
}

void UDashGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (bIsEndingAbility)
	{
		return;
	}

	bIsEndingAbility = true;

	if (DashTask)
	{
		DashTask->OnTimedOut.RemoveAll(this);
		DashTask->OnTimedOutAndDestinationReached.RemoveAll(this);
		DashTask->EndTask();
		DashTask = nullptr;
	}

	if (PlayMontageTask)
	{
		PlayMontageTask->OnCompleted.RemoveAll(this);
		PlayMontageTask->OnInterrupted.RemoveAll(this);
		PlayMontageTask->OnCancelled.RemoveAll(this);
		PlayMontageTask->EndTask();
		PlayMontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDashGameplayAbility::OnDashFinished()
{
	TerminateAbility(false);
}

void UDashGameplayAbility::OnMontageCompleted()
{
	// 의도적으로 비워둠.
	// 대시 종료는 DashTask->OnFinish 하나로만 처리.
}

void UDashGameplayAbility::OnMontageInterrupted()
{
	TerminateAbility(true);
}

void UDashGameplayAbility::OnMontageCancelled()
{
	TerminateAbility(true);
}

void UDashGameplayAbility::TerminateAbility(bool bCancelled)
{
	if (bIsEndingAbility)
	{
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bCancelled);
}

FVector UDashGameplayAbility::ResolveDashDirection(const FGameplayEventData* TriggerEventData) const
{
	if (TriggerEventData)
	{
		FVector Dir = FRotator(0.f, TriggerEventData->EventMagnitude, 0.f).Vector();
		Dir.Z = 0.f;
		return Dir.GetSafeNormal();
	}

	if (const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		FVector Dir = Character->GetActorForwardVector();
		Dir.Z = 0.f;
		return Dir.GetSafeNormal();
	}

	return FVector::ForwardVector;
}
