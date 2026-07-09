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

	// BlueprintNativeEvent라 BP 서브클래스에서 오버라이드해 플레인 메쉬 표시/애니메이션 등을
	// 덧붙일 수 있다(Call Parent Function으로 콜리전 토글은 그대로 유지 가능).
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Door")
	void CloseDoor();
	virtual void CloseDoor_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Door")
	void OpenDoor();
	virtual void OpenDoor_Implementation();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UBoxComponent> DoorBox;

	// 레벨 배치 시 시작부터 닫혀있게 하고 싶으면 켠다. 기본은 열림(false).
	UPROPERTY(EditAnywhere, Category = "Door")
	bool bStartClosed = false;

private:
	UFUNCTION()
	void OnRep_Closed();

	UPROPERTY(ReplicatedUsing = OnRep_Closed)
	bool bClosed = false;

	void ApplyDoorState();
};
