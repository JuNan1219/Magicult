// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicultCharacterBase.h"
#include "MagicultCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
/**
 *
 */
UCLASS()
class MAGICULT_API AMagicultCharacter : public AMagicultCharacterBase
{
	GENERATED_BODY()
public:
	AMagicultCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** Combat Interface */
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual FName GetDefaultKnockbackBone() const override;
	virtual void Die() override;
	/** end Combat Interface */

	/*
	* 테스트용 즉석 부활: 사망 후 일정 시간 뒤 죽은 자리에서 되살아난다.
	*/

	// false면 기존처럼 사망 상태로 남는다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Respawn")
	bool bTestRespawnEnabled = true;

	// 부활까지 대기 시간(초). BP 기본값 또는 런타임에 콘솔변수 magicult.TestRespawn.Delay 로 바꿀 수 있다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test|Respawn")
	float RespawnDelay = 10.f;

	// 부활 타이머가 걸려 있는가(서버 기준). 전멸 판정에서 "곧 돌아올 플레이어"를 걸러내는 데 쓴다.
	UFUNCTION(BlueprintPure, Category = "Test|Respawn")
	bool IsAwaitingRespawn() const;

	// 예약된 부활을 취소한다(예: 매치가 끝나 결과 화면이 뜬 뒤).
	void CancelPendingRespawn();

protected:
	virtual void InitializeDefaultAttributes() const;

	// 인덱스 0 = 1P 파란색, 인덱스 1 = 2P 빨간색
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
	TObjectPtr<UMaterialInterface> Player1Material;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Appearance")
	TObjectPtr<UMaterialInterface> Player2Material;

private:
	virtual void InitAbilityActorInfo() override;

	// PlayerState로부터 받아 복제됨
	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex)
	int32 PlayerIndex = 0;

	UFUNCTION()
	void OnRep_PlayerIndex();

	void ApplyPlayerColor();

	virtual void RestoreDefaultMaterials() override;

	// 콘솔변수 오버라이드가 있으면 그 값, 없으면 RespawnDelay를 쓴다.
	float GetEffectiveRespawnDelay() const;

	void OnRespawnTimerFinished();

	FTimerHandle RespawnTimerHandle;
};
