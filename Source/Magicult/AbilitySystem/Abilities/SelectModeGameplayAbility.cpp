// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectModeGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Magicult/Data/InputToTagConfig.h"
#include "Magicult/Data/MagicInfo.h"
#include "Magicult/Player/MagicultPlayerController.h"


USelectModeGameplayAbility::USelectModeGameplayAbility()
{
	// NetExecutionPolicy는 "입력→서버 라우팅"을 위해서만 LocalPredicted로 둔다.
	// (GameplayEvent 트리거는 로컬에서만 처리되므로, 클라 입력이 서버에 닿으려면 LocalPredicted가 필요하다.)
	// 실제 예측은 하지 않는다: 선택 상태 GE 적용은 ActivateAbility에서 HasAuthority로 가드되어
	// 서버에서만 적용되고 클라로 복제된다. → 클라 WidgetController 델리게이트는 서버 확정 시점에만 발동.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	// 같은 카테고리 모드를 다시 눌렀을 때 이미 활성 상태여도 재활성되어 GE가 교체되도록 허용한다.
	bRetriggerInstancedAbility = true;
}

void USelectModeGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!InputToTag)
	{
		UE_LOG(LogTemp, Warning, TEXT("You need to set InputToTag %s"), *ActorInfo->AvatarActor->GetName());
		return;
	}
	// 선택 상태 GE는 서버에서만 적용한다(무예측). 복제로 클라 ASC에 태그가 도착하면
	// WidgetController 델리게이트가 서버 확정 시점에 한 번 발동해 UI가 갱신된다.
	if (HasAuthority(&ActivationInfo) && TriggerEventData && InputToTag)
	{
		bool bFound = false;
		const FInputToSelect InputToSelect = InputToTag->FindTagToInput(TriggerEventData->EventTag, bFound);

        if (bFound && InputToSelect.EffectClass)
        {
            // 재활성(모드 재선택)으로 ActivateAbility가 다시 호출되면 이전 선택 GE를 먼저 제거해
            // 핸들 덮어쓰기로 인한 GE 누수를 막는다.
            if (AppliedEffectHandle.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
            {
                ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(AppliedEffectHandle);
                AppliedEffectHandle.Invalidate();
            }

            const FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
            const FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(InputToSelect.EffectClass, 1, ContextHandle);
            AppliedEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
        }
	}
}

void USelectModeGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AppliedEffectHandle.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(AppliedEffectHandle);
	}
}
