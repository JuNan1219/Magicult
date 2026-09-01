// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultEnemy.h"
#include "MagicultBoss.generated.h"

class UBossCombatComponent;

/**
 * 3종 패턴(Swing/Dash/JumpSmash)을 점수 기반으로 선택·실행하는 보스.
 *
 * AMagicultEnemy의 매치 초기화 / BT 시작 / HitReact·Dead 미러링 플럼빙을 그대로 물려받고,
 * 패턴 상태 관리를 담당하는 UBossCombatComponent만 추가한다.
 * 전투 로직 자체는 서버 권한(BT)에서 컴포넌트를 통해 돈다.
 */
UCLASS()
class MAGICULT_API AMagicultBoss : public AMagicultEnemy
{
	GENERATED_BODY()

public:
	AMagicultBoss();

	UBossCombatComponent* GetBossCombatComponent() const { return BossCombatComponent; }

	// 사망 처리(서버) 후 모든 머신에 처치 사실을 알린다.
	virtual void Die() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UBossCombatComponent> BossCombatComponent;

	// 테스트용 처치 알림. 켜져 있으면 보스가 죽을 때 모든 머신 화면에 메시지를 띄운다.
	// 정식 연출(위젯/컷신)이 붙으면 끄고 OnBossDefeated 이벤트만 쓰면 된다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Debug")
	bool bAnnounceDeathOnScreen = true;

	// 알림에 표시할 이름. 비어 있으면 액터 이름을 쓴다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Debug")
	FText BossDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Debug", meta = (ClampMin = "0.1"))
	float AnnounceDuration = 5.f;

	// 화면 메시지 대신(또는 함께) 위젯 연출을 붙이고 싶을 때 BP에서 구현한다. 모든 머신에서 호출된다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss")
	void OnBossDefeated();

private:
	// 사망은 서버에서만 처리되므로(AMagicultCharacterBase::Die), 알림은 멀티캐스트로 내려보낸다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastAnnounceDeath();
};
