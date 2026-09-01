// Fill out your copyright notice in the Description page of Project Settings.

#include "MagicultBoss.h"
#include "Magicult/AI/Boss/BossCombatComponent.h"
#include "Magicult/Game/MagicultGameModeBase.h"
#include "Engine/Engine.h"

AMagicultBoss::AMagicultBoss()
{
	BossCombatComponent = CreateDefaultSubobject<UBossCombatComponent>("BossCombatComponent");
}

void AMagicultBoss::Die()
{
	// 부모의 사망 게이트(권한 없음 / 이미 사망)에 걸리면 알림도 나가면 안 되므로,
	// 호출 전후의 bDead를 비교해 "이번 호출이 실제로 죽인 경우"에만 멀티캐스트한다.
	const bool bWasDead = bDead;

	Super::Die();

	if (!HasAuthority() || bWasDead || !bDead) return;

	MulticastAnnounceDeath();

	// 승리 판정은 GameMode가 한다(살아있는 보스가 더 없으면 매치 종료).
	if (AMagicultGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AMagicultGameModeBase>())
	{
		GameMode->NotifyBossDefeated(this);
	}
}

void AMagicultBoss::MulticastAnnounceDeath_Implementation()
{
	const FString DisplayName = BossDisplayName.IsEmpty() ? GetName() : BossDisplayName.ToString();
	const FString Message = FString::Printf(TEXT("보스 처치! (%s)"), *DisplayName);

	UE_LOG(LogTemp, Log, TEXT("%s"), *Message);

	if (bAnnounceDeathOnScreen && GEngine)
	{
		// 키를 고정해 여러 보스가 연달아 죽어도 줄이 쌓이지 않게 한다(같은 슬롯을 덮어씀).
		GEngine->AddOnScreenDebugMessage(/*Key*/ 7701, AnnounceDuration, FColor::Yellow, Message);
	}

	OnBossDefeated();
}
