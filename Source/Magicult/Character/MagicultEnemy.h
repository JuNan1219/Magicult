// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultCharacterBase.h"
#include "Magicult/UI/WidgetController/OverlayWidgetController.h"
#include "Magicult//Data/CharacterClassInfo.h"
#include "Magicult/Interface/EnemyInterface.h"
#include "MagicultEnemy.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class AMagicultAIController;
class AController;

/**
 * 
 */
UCLASS()
class MAGICULT_API AMagicultEnemy : public AMagicultCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:
	AMagicultEnemy();
	virtual void PossessedBy(AController* NewController);

	// 매치 시작 시 GameMode가 호출(서버). 인원수 체력 배율로 속성을 초기화하고 AI(BT)를 시작한다.
	// 몬스터 속성/AI 초기화를 BeginPlay가 아닌 이 시점으로 모아, 레벨 배치 몬스터도
	// 전원 입장이 확정된 뒤에 인원수에 맞게 초기화되도록 한다.
	void InitializeForMatch(float HealthMultiplier);

	// bWaitForActivationTrigger가 true인 경우, InitializeForMatch가 AI를 자동으로 켜지 않고
	// 이 함수가 호출될 때(예: 보스방 트리거)까지 기다린다. 속성/어빌리티는 이미 준비된 상태.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ActivateCombat();

	/** Combat interface */
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void Die() override;
	/** End Combat interface */
	/** Enemy interface */
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	/** End Enemy interface */

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

	UPROPERTY(BlueprintAssignable)
	FOnFloatChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnFloatChangedSignature OnMaxHealthChanged;

	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5;

	// 타겟 탐지 최대 거리. 0 이하면 무제한(보스방 몬스터 등 항상 타겟을 잡아야 하는 경우).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float MaxDetectRange = 1500.f;

	// true면 InitializeForMatch에서 BT를 자동으로 켜지 않고 ActivateCombat() 호출을 기다린다.
	// 보스방 트리거로 전투를 시작시키고 싶은 보스에 켠다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bWaitForActivationTrigger = false;
protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;

	// Debuff.Stunned를 Blackboard로 미러링해 BT가 데코레이터(Observer Aborts)로 중단할 수 있게 한다.
	virtual void OnStunnedChanged(bool bNewStunned) override;

	// BT 시작을 한 곳으로 모은다. PossessedBy/InitializeForMatch 호출 순서와 무관하게
	// 매치가 시작되고 컨트롤러가 준비됐을 때만 실제로 실행된다.
	void StartBehaviorTree();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Slime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<AMagicultAIController> MagicultAIController;

	// 매치가 시작되어 InitializeForMatch가 호출됐는지. 매치 시작 전 AI 작동을 막는다.
	bool bMatchStarted = false;
};
