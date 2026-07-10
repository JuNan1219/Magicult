// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorActor.generated.h"

class UBoxComponent;

/**
 * 보스방 입구 등에 배치하는 박스 형태의 문. 레벨에서 위치/크기를 자유롭게 옮길 수 있도록
 * BoxComponent 하나만 루트로 갖는다. BossTriggerActor 등이 CloseDoor/OpenDoor를 호출해 사용한다.
 */
UCLASS()
class MAGICULT_API ADoorActor : public AActor
{
	GENERATED_BODY()

public:
	ADoorActor();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void CloseDoor();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenDoor();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UBoxComponent> DoorBox;

	// 레벨 배치 시 시작부터 닫혀있게 하고 싶으면 켠다. 기본은 열림(false).
	UPROPERTY(EditAnywhere, Category = "Door")
	bool bStartClosed = false;

	// 콜리전 토글 이후 항상 호출된다 — BeginPlay(bStartClosed), OnRep_Closed(리모트 클라 동기화),
	// CloseDoor/OpenDoor(명시적 호출) 세 경로 전부 여기로 모여서 BP가 하나의 이벤트만 오버라이드하면
	// 어느 경로로 상태가 바뀌든 비주얼(플레인 등)이 항상 따라온다.
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorStateChanged(bool bNewClosed);
private:
	UFUNCTION()
	void OnRep_Closed();

	UPROPERTY(ReplicatedUsing = OnRep_Closed)
	bool bClosed = false;

	void ApplyDoorState();
};
