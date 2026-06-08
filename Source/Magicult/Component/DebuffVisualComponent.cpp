// Fill out your copyright notice in the Description page of Project Settings.

#include "DebuffVisualComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Magicult/MagicultGameplayTags.h"

UDebuffVisualComponent::UDebuffVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // 활성 디버프 있을 때만 켜짐
}

void UDebuffVisualComponent::BeginPlay()
{
	Super::BeginPlay();
	TryInitializeWithASC();
}

void UDebuffVisualComponent::TryInitializeWithASC()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Owner);
	if (!ASCInterface) return;

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		// 플레이어는 PlayerState에서 ASC가 늦게 초기화되므로 다음 틱에 재시도
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UDebuffVisualComponent::TryInitializeWithASC);
		return;
	}

	CachedASC = ASC;

	for (const FDebuffVisualEntry& Entry : DebuffVisuals)
	{
		if (!Entry.DebuffTag.IsValid()) continue;

		FDelegateHandle Handle = ASC->RegisterGameplayTagEvent(
			Entry.DebuffTag,
			EGameplayTagEventType::NewOrRemoved
		).AddUObject(this, &UDebuffVisualComponent::OnDebuffTagChanged);

		TagEventHandles.Add(Entry.DebuffTag, Handle);
	}
}

void UDebuffVisualComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 등록한 태그 이벤트 핸들 해제
	if (CachedASC.IsValid())
	{
		for (auto& [Tag, Handle] : TagEventHandles)
		{
			CachedASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).Remove(Handle);
		}
	}
	TagEventHandles.Empty();

	Super::EndPlay(EndPlayReason);
}

void UDebuffVisualComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 활성 디버프가 있을 때만 매 프레임 빌보드 방향으로 위치 갱신
	if (ActiveDebuffOrder.Num() > 0)
	{
		RefreshLayout();
	}
}

void UDebuffVisualComponent::OnDebuffTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		AddDebuffVisual(Tag);
	}
	else
	{
		RemoveDebuffVisual(Tag);
	}
}

void UDebuffVisualComponent::AddDebuffVisual(const FGameplayTag& Tag)
{
	// 중복 생성 방지
	if (ActiveDebuffComponents.Contains(Tag)) return;

	// 전용 NiagaraSystem 검색
	const FDebuffVisualEntry* Entry = DebuffVisuals.FindByPredicate([&Tag](const FDebuffVisualEntry& E)
	{
		return E.DebuffTag == Tag;
	});

	if (!Entry || !Entry->NiagaraSystem) return;

	// 전용 서버에서는 비주얼 생성 불필요
	if (IsNetMode(NM_DedicatedServer)) return;

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		Entry->NiagaraSystem,
		this,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		false // bAutoDestroy = false, 수동 관리
	);

	if (!NiagaraComp) return;

	ActiveDebuffComponents.Add(Tag, NiagaraComp);
	ActiveDebuffOrder.AddUnique(Tag);

	// 첫 디버프 추가 시 Tick 활성화
	if (ActiveDebuffOrder.Num() == 1)
	{
		SetComponentTickEnabled(true);
	}

	RefreshLayout();
	OnDebuffAdded(Tag);
}

void UDebuffVisualComponent::RemoveDebuffVisual(const FGameplayTag& Tag)
{
	TObjectPtr<UNiagaraComponent>* CompPtr = ActiveDebuffComponents.Find(Tag);
	if (!CompPtr || !*CompPtr) return;

	(*CompPtr)->DestroyComponent();
	ActiveDebuffComponents.Remove(Tag);
	ActiveDebuffOrder.Remove(Tag);

	// 모든 디버프 제거 시 Tick 비활성화
	if (ActiveDebuffOrder.Num() == 0)
	{
		SetComponentTickEnabled(false);
	}
	else
	{
		RefreshLayout();
	}

	OnDebuffRemoved(Tag);
}

void UDebuffVisualComponent::RefreshLayout()
{
	int32 Count = ActiveDebuffOrder.Num();
	if (Count == 0) return;

	// 카메라 우측 벡터 (빌보드 정렬 기준축)
	FVector RightVector = FVector::RightVector;
	if (APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
	{
		FVector CamRight = FRotationMatrix(CamMgr->GetCameraRotation()).GetScaledAxis(EAxis::Y);
		CamRight.Z = 0.f;
		if (!CamRight.IsNearlyZero())
		{
			RightVector = CamRight.GetSafeNormal();
		}
	}

	// 가운데 정렬: Count=1→오프셋 0, Count=2→(-Spacing/2, +Spacing/2), ...
	float TotalWidth = (Count - 1) * Spacing;
	float StartOffset = -TotalWidth * 0.5f;
	FVector Origin = GetComponentLocation();

	for (int32 i = 0; i < Count; ++i)
	{
		TObjectPtr<UNiagaraComponent>* CompPtr = ActiveDebuffComponents.Find(ActiveDebuffOrder[i]);
		if (!CompPtr || !*CompPtr) continue;

		float Offset = StartOffset + i * Spacing;
		(*CompPtr)->SetWorldLocation(Origin + RightVector * Offset);
	}
}
