// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossTriggerActor.generated.h"

class UBoxComponent;
class ADoorActor;
class AMagicultEnemy;

/**
 * 보스방 입구에 배치하는 박스 트리거. 플레이어가 겹치면 문을 닫고 보스 AI를 켠다.
 * 위치/크기를 자유롭게 옮길 수 있도록 BoxComponent 하나만 루트로 갖는다.
 */
UCLASS()
class MAGICULT_API ABossTriggerActor : public AActor
{
	GENERATED_BODY()

public:
	ABossTriggerActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Trigger")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, Category = "Boss Trigger")
	TArray<TObjectPtr<ADoorActor>> DoorsToClose;

	UPROPERTY(EditAnywhere, Category = "Boss Trigger")
	TArray<TObjectPtr<AMagicultEnemy>> BossesToActivate;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bTriggered = false;
};
