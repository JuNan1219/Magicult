#include "MagicultDamageGameplayAbility.h"
#include "Magicult/Attack/AttackInstance.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Actor.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Magicult/Attack/AttackDefinition.h"
#include "Magicult/MagicultGameplayTags.h"

UMagicultDamageGameplayAbility::UMagicultDamageGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	bRetriggerInstancedAbility = false;
	ActivationBlockedTags.AddTag(FMagicultGameplayTags::Get().Debuff_Stunned);
	ActivationBlockedTags.AddTag(FMagicultGameplayTags::Get().Character_State_Dead);
}

void UMagicultDamageGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bIsEndingAbility = false;

	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* SourceActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!SourceActor || !SourceActor->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FAttackDefinition AttackDefinition = GetRandomAttackDefinitionFromArray(
		ICombatInterface::Execute_GetAttackDefinitions(SourceActor, TriggerEventData->EventTag));

	if (!AttackDefinition.AttackInstanceClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CurrentAttackDefinition = AttackDefinition;

	// Task 등록 먼저 — Execute()가 이벤트를 즉시 발행할 수 있으므로 반드시 선행
	WaitGameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FMagicultGameplayTags::Get().Notify_Attack, nullptr, false, false);
	if (WaitGameplayEventTask)
	{
		WaitGameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::HandleMontageGameplayEvent);
		WaitGameplayEventTask->ReadyForActivation();
	}

	if (AttackDefinition.Montage)
	{
		PlayMontageAndWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("AttackMontage"), AttackDefinition.Montage, 1.0f, NAME_None, true, 1.0f);
		if (!PlayMontageAndWaitTask)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		PlayMontageAndWaitTask->OnCompleted.AddDynamic(this, &ThisClass::HandleMontageCompleted);
		PlayMontageAndWaitTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleMontageInterrupted);
		PlayMontageAndWaitTask->OnCancelled.AddDynamic(this, &ThisClass::HandleMontageCancelled);
		PlayMontageAndWaitTask->ReadyForActivation();
	}
	else
	{
		// 몽타주 없을 때 타임아웃으로 GA 종료
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				AbilityTimeoutHandle, this,
				&ThisClass::HandleAbilityTimeout,
				AbilityTimeout, false);
		}
	}

	// AttackInstance는 전투 판정의 실행부라 서버에서만 만든다. 클라는 몽타주만 재생하고
	// 연출은 서버가 쏜 GameplayCue로 받는다.
	if (ActorInfo->IsNetAuthority())
	{
		CurrentAttackInstance = NewObject<UAttackInstance>(this, AttackDefinition.AttackInstanceClass);
		if (!CurrentAttackInstance)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		CurrentAttackInstance->SetOwnerActor(ActorInfo->AvatarActor.Get());
		CurrentAttackInstance->Initialize(
			UMagicultAbilitySystemLibrary::MakeSpellContext(
				GetAbilitySystemComponentFromActorInfo(), nullptr,
				GetAvatarActorFromActorInfo(), nullptr,
				FHitResult(), false, CurrentAttackInstance),
			CurrentAttackDefinition.AttackSpec,
			CurrentAttackDefinition.bOverrideAttackSpec);
		CurrentAttackInstance->SetInstanceConfig(CurrentAttackDefinition.InstanceConfig);
		if (CurrentAttackDefinition.SocketTag.IsValid())
			CurrentAttackInstance->SetSocketTag(CurrentAttackDefinition.SocketTag);

		CurrentAttackInstance->Execute();
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMagicultDamageGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	bIsEndingAbility = true;

	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(AbilityTimeoutHandle);

	// 이벤트 수신 즉시 차단
	if (WaitGameplayEventTask)
	{
		WaitGameplayEventTask->EndTask();
		WaitGameplayEventTask = nullptr;
	}

	CleanupAttackInstance();

	// PlayMontageAndWaitTask는 직접 EndTask하지 않음
	// Super::EndAbility → EndActiveTasks → OnDestroy(AbilityEnded=true) → StopPlayingMontage
	PlayMontageAndWaitTask = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ---------------------------------------------------------------

void UMagicultDamageGameplayAbility::TerminateAbility(bool bCancelled)
{
	if (bIsEndingAbility) return;
	bIsEndingAbility = true;

	CleanupAttackInstance();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bCancelled);
}

void UMagicultDamageGameplayAbility::CleanupAttackInstance()
{
	if (!CurrentAttackInstance) return;

	CurrentAttackInstance->Cancel();
	CurrentAttackInstance = nullptr;
}

// ---------------------------------------------------------------
// 핸들러
// ---------------------------------------------------------------

// 몽타주 AnimNotify → SendGameplayEventToActor → WaitGameplayEvent 경로 전용.
// 그 외(Task 델리게이트 등)는 CurrentAttackInstance->HandleEventTag() 직접 호출.
void UMagicultDamageGameplayAbility::HandleMontageGameplayEvent(FGameplayEventData Payload)
{
	if (CurrentAttackInstance)
	{
		CurrentAttackInstance->HandleEventTag(Payload.EventTag);
	}
}

void UMagicultDamageGameplayAbility::HandleMontageCompleted()   { TerminateAbility(false); }
void UMagicultDamageGameplayAbility::HandleMontageInterrupted() { TerminateAbility(true);  }
void UMagicultDamageGameplayAbility::HandleMontageCancelled()   { TerminateAbility(true);  }
void UMagicultDamageGameplayAbility::HandleAbilityTimeout()     { TerminateAbility(false); }

// ---------------------------------------------------------------

FAttackDefinition UMagicultDamageGameplayAbility::GetRandomAttackDefinitionFromArray(const TArray<FAttackDefinition>& AttackDefinitions) const
{
	if (AttackDefinitions.Num() > 0)
	{
		return AttackDefinitions[FMath::RandRange(0, AttackDefinitions.Num() - 1)];
	}
	return FAttackDefinition();
}
