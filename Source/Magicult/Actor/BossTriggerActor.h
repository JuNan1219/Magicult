// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossTriggerActor.generated.h"

class ADoorActor;
class AMagicultEnemy;
class UBoxComponent;

/**
 * 문 열기/닫기 + 몬스터 활성화를 실행하는 트리거. 발동 조건은 인스턴스별로 켜고 끌 수 있다 —
 * bActivateOnPlayerOverlap을 켜면 박스에 플레이어가 겹칠 때 자동 발동하고, MonstersToClear를
 * 채우면 등록된 몬스터가 전부 죽었을 때(방 클리어) 자동 발동한다. 둘 다 켜도 되고, 아예 끄고
 * 외부에서 Activate()를 직접 호출해도 된다.
 */
UCLASS()
class MAGICULT_API ABossTriggerActor : public AActor
{
	GENERATED_BODY()

public:
	ABossTriggerActor();

	// 문 열기/닫기 + EnemiesToActivate 활성화를 실행한다. 한 번만 발동한다(재호출 시 무시).
	UFUNCTION(BlueprintCallable, Category = "Boss Trigger")
	void Activate();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Trigger")
	TObjectPtr<UBoxComponent> TriggerBox;

	// true면 TriggerBox에 플레이어가 겹칠 때 자동으로 Activate()가 호출된다.
	// false면 TriggerBox는 콜리전이 꺼진 채로 남고(에디터에서 위치/크기만 참고용), 오버랩으로는 발동하지 않는다.
	UPROPERTY(EditAnywhere, Category = "Boss Trigger")
	bool bActivateOnPlayerOverlap = false;

	UPROPERTY(EditAnywhere, Category = "Boss Trigger")
	TArray<TObjectPtr<ADoorActor>> DoorsToClose;

	UPROPERTY(EditAnywhere, Category = "Boss Trigger")
	TArray<TObjectPtr<ADoorActor>> DoorsToOpen;

	UPROPERTY(EditAnywhere, Category = "Boss Trigger")
	TArray<TObjectPtr<AMagicultEnemy>> EnemiesToActivate;

	// 비어있지 않으면, 여기 등록된 몬스터가 전부 죽었을 때 자동으로 Activate()가 호출된다(방 클리어 조건).
	UPROPERTY(EditAnywhere, Category = "Boss Trigger")
	TArray<TObjectPtr<AMagicultEnemy>> MonstersToClear;

private:
	UFUNCTION()
	void OnTrackedMonsterDied(AMagicultEnemy* DeadEnemy);

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool bTriggered = false;
	int32 RemainingToClear = 0;
};
