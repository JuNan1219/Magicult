// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectModeGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Magicult/Data/InputToTagConfig.h"
#include "Magicult/Data/MagicInfo.h"
#include "Magicult/Player/MagicultPlayerController.h"
#include "Magicult/MagicultGameplayTags.h"


USelectModeGameplayAbility::USelectModeGameplayAbility()
{
	// 모드 태그(Character.Mode.*)는 비복제 loose 태그다. 서버와 소유 클라가 각자 자기 ASC에만
	// 붙이고(ActivateAbility) 자기 ASC에서만 뗀다(EndAbility) — 서로에게 복제하지 않는다.
	//
	// 그렇게 하는 이유: 이 태그가 켜져 있는 시간(모드 선택~디테일 선택)이 RTT보다 짧을 수 있다.
	// GameplayEffect로 부여해 복제에 맡기면, 클라가 이미 로컬 예측으로 태그를 껐다 켰다 하는
	// 동안 서버의 권위 있는 과거 기록(GE 적용/제거)이 뒤늦게 도착해 유령처럼 다시 켜졌다 꺼진다
	// (Docs/plans/idea_selectmode_mode_tag_ghost.md 원인 분석 참고). loose 태그는 복제 경로 자체가
	// 없어 이 유령이 원리적으로 발생하지 않는다.
	//
	// ⚠️ AddReplicatedLooseGameplayTag는 쓰면 안 된다 — 이름만 다르고 복제 경로가 그대로 살아나
	// 같은 유령이 재현된다.
	//
	// 수명 관리는 이 클래스의 EndAbility 하나뿐이고, 별도 감시 로직이 필요 없다. 정상 종료(디테일
	// GA의 CancelAbilitiesWithTag=Character.Mode)와 인터럽트 종료(GA_MagicMode::EndAbility의
	// CancelAbilitiesWithTag(Character.Mode) 캐스케이드, 사망도 MagicultCharacterBase::Die()가
	// 같은 캐스케이드를 태우도록 되어 있음) 둘 다 결국 이 어빌리티를 서버·클라 각자의 로컬 ASC
	// 기준으로 캔슬하며, 그 캔슬은 항상 EndAbility를 거친다 — 정리가 양쪽에서 자체 완결된다
	// (Docs/plans/plan_selectmode_loose_mode_tag.md 배경 절 참고).
	//
	// NetExecutionPolicy = LocalPredicted + ActivateAbility의 (HasAuthority || IsLocallyControlled)
	// 가드로 실제 예측을 한다. MagicSetupGameplayAbility.cpp(디테일 선택)가 이미 같은 패턴으로
	// RTT 없이 동작하고 있고, 이 어빌리티도 이제 순수 로컬 태그 조작이라 그것과 완전히 대칭이다.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	// 같은 카테고리 모드를 다시 눌렀을 때 이미 활성 상태여도 재활성되어 GE가 교체되도록 허용한다.
	bRetriggerInstancedAbility = true;
	ActivationBlockedTags.AddTag(FMagicultGameplayTags::Get().Debuff_Stunned);
}

void USelectModeGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!InputToTag)
	{
		UE_LOG(LogTemp, Warning, TEXT("You need to set InputToTag %s"), *ActorInfo->AvatarActor->GetName());
		return;
	}
	// 서버 + 소유 클라 양쪽에서 각자 로컬로 실행한다(진짜 예측). InputToTag는 순수 함수라
	// 같은 DataAsset을 참조하는 양쪽이 같은 입력에 대해 결정론적으로 같은 결과를 낸다.
	//
	// 이 가드는 지금 시점 기준 사실상 항상 참이다 — NetExecutionPolicy::LocalPredicted +
	// InstancedPerActor 조합에서 ActivateAbility 본문은 서버 또는 소유(로컬 컨트롤) 클라
	// 외의 머신(예: 다른 플레이어를 관전 중인 제3자 클라)에서는 애초에 실행되지 않는다 —
	// GAS가 어빌리티 로직 실행과 결과 관찰(리플리케이션)을 분리하기 때문이다. 그래도
	// MagicSetupGameplayAbility.cpp의 기존 패턴과 일관성을 맞추고 방어적으로 남겨둔다.
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
