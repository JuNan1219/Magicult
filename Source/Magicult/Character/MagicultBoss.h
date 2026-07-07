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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	TObjectPtr<UBossCombatComponent> BossCombatComponent;
};
