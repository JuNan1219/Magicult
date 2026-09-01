// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectModeGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Magicult/Data/InputToTagConfig.h"
#include "Magicult/Data/MagicInfo.h"
#include "Magicult/Player/MagicultPlayerController.h"
#include "Magicult/MagicultGameplayTags.h"


USelectModeGameplayAbility::USelectModeGameplayAbility()
{
	// 모드 태그(Character.Mode.*)는 비복제 loose 태그로, 서버/소유 클라가 각자 자기 ASC에만
	// 적용·해제한다. GE로 복제에 맡기면 태그 on 구간이 RTT보다 짧을 때 서버의 지연 도착한
	// 과거 기록이 클라의 로컬 예측을 덮어써 유령처럼 재발동하는 문제가 있었다
	// (AddReplicatedLooseGameplayTag도 복제 경로가 살아있어 동일 문제 재현 — 사용 금지).
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	// 같은 카테고리 모드를 다시 눌렀을 때 이미 활성 상태여도 재활성되어 GE가 교체되도록 허용한다.
	bRetriggerInstancedAbility = true;
	ActivationBlockedTags.AddTag(FMagicultGameplayTags::Get().Debuff_Stunned);
	ActivationBlockedTags.AddTag(FMagicultGameplayTags::Get().Character_State_Dead);
}

void USelectModeGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!InputToTag)
	{
		UE_LOG(LogTemp, Warning, TEXT("You need to set InputToTag %s"), *ActorInfo->AvatarActor->GetName());
		return;
	}
	// 서버 + 소유 클라 양쪽에서 각자 로컬로 실행된다(예측). InputToTag가 순수 함수라 양쪽이
	// 같은 입력에 결정론적으로 같은 결과를 낸다. HasAuthority||IsLocallyControlled 가드는
	// GAS 특성상 사실상 항상 참이지만 MagicSetupGameplayAbility.cpp와 패턴을 맞추려고 유지한다.
	if ((HasAuthority(&ActivationInfo) || ActorInfo->IsLocallyControlled()) && TriggerEventData && InputToTag)
	{
		bool bFound = false;
		const FInputToSelect InputToSelect = InputToTag->FindTagToInput(TriggerEventData->EventTag, bFound);

		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (bFound && InputToSelect.SelectedTag.IsValid() && ASC)
		{
			// 재활성(모드 재선택)으로 다시 들어오면 이전 모드 태그를 먼저 뺀다.
			// loose 태그는 참조 카운트라 빼지 않고 덮어쓰면 이전 태그가 영구 잔류한다.
			if (AppliedModeTag.IsValid())
			{
				ASC->RemoveLooseGameplayTag(AppliedModeTag);
			}

			AppliedModeTag = InputToSelect.SelectedTag;
			ASC->AddLooseGameplayTag(AppliedModeTag);
		}
	}
}

void USelectModeGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (AppliedModeTag.IsValid())
	{
		if (UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
		{
			ASC->RemoveLooseGameplayTag(AppliedModeTag);
		}
		// ASC가 이미 사라진 경우에도 상태는 반드시 초기화한다.
		// InstancedPerActor라 이 인스턴스는 재사용되며, 유효한 태그가 남아 있으면
		// 다음 활성화의 "이전 태그 제거"가 존재하지 않는 태그를 빼려 든다.
		AppliedModeTag = FGameplayTag();
	}
}
