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
	bRetriggerInstancedAbility = false;
}

void UMagicultDamageGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bIsEndingAbility = false;


	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* SourceActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!SourceActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!SourceActor->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const TArray<FAttackDefinition> AttackDefinitions = ICombatInterface::Execute_GetAttackDefinitions(SourceActor, TriggerEventData->EventTag);
	const FAttackDefinition AttackDefinition = GetRandomAttackDefinitionFromArray(AttackDefinitions);

	if (!AttackDefinition.AttackInstanceClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	CurrentAttackInstance = NewObject<UAttackInstance>(this, AttackDefinition.AttackInstanceClass);
	if (!CurrentAttackInstance)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CurrentAttackInstance->SetOwnerActor(ActorInfo->AvatarActor.Get());

	FSpellContext SpellContext = UMagicultAbilitySystemLibrary::MakeSpellContext(
		GetAbilitySystemComponentFromActorInfo(),
		nullptr,
		GetAvatarActorFromActorInfo(),
		nullptr,
		FHitResult(),
		false,
		CurrentAttackInstance
	);

	CurrentAttackDefinition = AttackDefinition;

	CurrentAttackInstance->Initialize(SpellContext, CurrentAttackDefinition.AttackSpec, CurrentAttackDefinition.bOverrideAttackSpec);
	CurrentAttackInstance->SetInstanceConfig(CurrentAttackDefinition.InstanceConfig);
	if (CurrentAttackDefinition.SocketTag.IsValid())
		CurrentAttackInstance->SetSocketTag(CurrentAttackDefinition.SocketTag);

	CurrentAttackInstance->OnFinished.AddUObject(this, &ThisClass::HandleAttackInstanceFinished);
	// 공격 인스턴스는 즉시 실행
	CurrentAttackInstance->Execute();

	// Notify 이벤트 대기 등록
	WaitGameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FMagicultGameplayTags::Get().Notify_Attack,
		nullptr,
		false,
		false
	);

	if (WaitGameplayEventTask)
	{
		WaitGameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::HandleMontageGameplayEvent);
		WaitGameplayEventTask->ReadyForActivation();
	}
	
	if (AttackDefinition.Montage)
	{
		

		// 몽타주 재생
		PlayMontageAndWaitTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,
				TEXT("AttackMontage"),
				AttackDefinition.Montage,
				1.0f,
				NAME_None,
				false,
				1.0f
			);
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
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMagicultDamageGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{

	if (PlayMontageAndWaitTask)
	{
		PlayMontageAndWaitTask->EndTask();
		PlayMontageAndWaitTask = nullptr;
	}

	if (WaitGameplayEventTask)
	{
		WaitGameplayEventTask->EndTask();
		WaitGameplayEventTask = nullptr;
	}

	if (CurrentAttackInstance)
	{
		CurrentAttackInstance->Cancel();
		CurrentAttackInstance = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMagicultDamageGameplayAbility::HandleMontageGameplayEvent(FGameplayEventData Payload)
{
	if (!CurrentAttackInstance)
	{
		return;
	}

	// Notify로 들어온 정보를 AttackInstance에 전달
	CurrentAttackInstance->HandleEventTag(Payload.EventTag);
}

void UMagicultDamageGameplayAbility::HandleAttackInstanceFinished(UAttackInstance* FinishedInstance)
{
	if (bIsEndingAbility)
	{
		return;
	}

	if (FinishedInstance != CurrentAttackInstance)
	{
		return;
	}

	bIsEndingAbility = true;
	CurrentAttackInstance = nullptr;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UMagicultDamageGameplayAbility::HandleMontageCompleted()
{
	if (bIsEndingAbility)
	{
		return;
	}

	bIsEndingAbility = true;

	if (CurrentAttackInstance)
	{
		CurrentAttackInstance->Finish();
		CurrentAttackInstance = nullptr;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UMagicultDamageGameplayAbility::HandleMontageInterrupted()
{
	if (bIsEndingAbility)
	{
		return;
	}

	bIsEndingAbility = true;

	if (CurrentAttackInstance)
	{
		CurrentAttackInstance->Cancel();
		CurrentAttackInstance = nullptr;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UMagicultDamageGameplayAbility::HandleMontageCancelled()
{
	if (bIsEndingAbility)
	{
		return;
	}

	bIsEndingAbility = true;

	if (CurrentAttackInstance)
	{
		CurrentAttackInstance->Cancel();
		CurrentAttackInstance = nullptr;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

FAttackDefinition UMagicultDamageGameplayAbility::GetRandomAttackDefinitionFromArray(const TArray<FAttackDefinition>& AttackDefinitions) const
{
	if (AttackDefinitions.Num() > 0)
	{
		const int32 Selection = FMath::RandRange(0, AttackDefinitions.Num() - 1);
		return AttackDefinitions[Selection];
	}

	return FAttackDefinition();
}
