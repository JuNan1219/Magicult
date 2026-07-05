// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackInstance.h"
#include "RadialAttackInstance.generated.h"

/**
 * 반경 오버랩으로 대상을 수집하고 타깃별 Trigger.Hit을 발행하는 논리 공격.
 * SpawnAttackInstance Action이 생성해 즉시 실행한다(Notify_Attack_Commit 시점에 수집).
 * 위치는 Location 정식 설계 전까지 Context에서 직접 읽는 임시 방식을 쓴다(ResolveOrigin).
 */
UCLASS()
class MAGICULT_API URadialAttackInstance : public UAttackInstance
{
	GENERATED_BODY()

public:
	virtual void HandleEventTag_Implementation(FGameplayTag EventTag) override;

	// 부모 Spec을 물려받을 때만 호출된다(Initialize의 bOverride=true 경로).
	// 폭발은 정지체이므로 딸려온 속도 기반 데미지를 끈다.
	virtual void SetAttackSpec(const FAttackSpec& InAttackSpec) override;

protected:
	virtual void InitializeWithConfig() override;

	bool TryCollectTargets(TArray<AActor*>& OutTargets) const;

protected:
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawDebug = false;

	float Radius = 0.f;
	bool bIgnoreOwner = false;
};
