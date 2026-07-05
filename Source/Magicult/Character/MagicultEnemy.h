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
protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;

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
