// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Magicult/Interface/MagicInterface.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/Attack/AttackDefinition.h"
#include "MagicultCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UMagicComponent;
class USkeletalMeshComponent;
class UMaterialInterface;
class UMaterialInstance;
class UMaterialInstanceDynamic;
class USkeletalMeshSocket;
struct FOnAttributeChangeData;

UCLASS(Abstract)
class MAGICULT_API AMagicultCharacterBase : public ACharacter, public IAbilitySystemInterface, public IMagicInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AMagicultCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/** Combat Interface */
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	virtual FTransform GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag) override;
	virtual bool AttachActorToSocket_Implementation(const FGameplayTag& SocketTag, AActor* Target) override;
	virtual void Die() override;
	virtual bool IsDead_Implementation() const override;
	virtual const TArray<FAttackDefinition> GetAttackDefinitions_Implementation(const FGameplayTag& InputTag) override;
	virtual FGameplayTag GetTeamTag_Implementation() const override;
	/** end Combat Interface */
	/** Magic Interface */
	virtual UMagicComponent* GetMagicComponent_Implementation() override;
	/** End Magic Interface */

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();

	// 사망 상태를 되돌린다(서버 권한). Die()의 대칭.
	// 죽은 자리로 되돌린 뒤 MulticastHandleRevive()로 각 머신의 비주얼/콜리전을 복구하고 바이탈을 리셋한다.
	virtual void Revive();

	// MulticastHandleDeath가 바꾼 상태를 모든 머신에서 원복한다.
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleRevive();

	// 사망(래그돌) 상태 넉백 impulse를 모든 머신에서 적용한다.
	// 래그돌 시뮬은 각 머신이 로컬로 돌리므로 impulse도 멀티캐스트해야 서버/클라 시체가 일관되게 날아간다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyRagdollImpulse(FVector Impulse, FName BoneName);

	// 피격 시 호출(서버). 전투 타이머 GE를 자신에게 적용해 RecentlyDamaged 태그를 갱신한다.
	// AttributeSet::HandleIncomingDamage에서 호출된다.
	void ApplyCombatTimeout();

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FTaggedAttackDefinitions> TaggedAttackDefinitions;

	// 피아식별 파벌 태그. BP 기본값에서 Character.Team.Player / Character.Team.Enemy 지정.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTag TeamTag;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName HeadSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName MagicSocketName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magic")
	TObjectPtr<UMagicComponent> MagicComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	// 무한+주기 리젠 GE를 서버에서 1회 적용한다. 속성 초기화 이후(자식 클래스의 ASC 초기화 지점) 호출한다.
	void ApplyHealthRegenEffect();

	virtual void InitializeDefaultAttributes() const;

	void AddCharacterAbilities();

	/*
	* Dissolve Effects
	*/
	void Dissolve();

	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	UPROPERTY(BlueprintReadOnly)
	bool bDead = false;

	/*
	* Revive (부활 시 원복용 기본값)
	*/

	// MulticastHandleDeath가 바꾸는 값들의 원본. BeginPlay에서 1회 캐시한다.
	void CacheReviveDefaults();

	// 디졸브로 교체된 머티리얼을 되돌린다. 자식이 자기 메시 머티리얼 복구를 덧붙인다.
	virtual void RestoreDefaultMaterials();

	// 서버에서 체력/마나를 최대치로 되돌린다. bDead 해제 이후에 호출해야 사망 게이트에 막히지 않는다.
	virtual void ResetVitalAttributes();

	FTransform DefaultMeshRelativeTransform;
	FTransform DefaultWeaponRelativeTransform;
	FName DefaultWeaponAttachSocket;
	FTransform DefaultSpringArmRelativeTransform;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> DefaultWeaponMaterial;

	UPROPERTY()
	TObjectPtr<USceneComponent> DefaultSpringArmParent;

	// 사망 시점 트랜스폼. 사망 후에도 캡슐은 밀릴 수 있으므로 부활은 이 값 기준으로 한다.
	FVector DeathLocation = FVector::ZeroVector;
	FRotator DeathRotation = FRotator::ZeroRotator;

	/*
	* Health Regen (전투 이탈 시 체력 회복)
	*/

	// true일 때만 리젠 GE 적용 + 피격 시 전투 타이머 GE(RecentlyDamaged 태그)를 적용한다. 캐릭터별 on/off용.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Regen")
	bool bEnableHealthRegen = true;

	// 무한+주기 GE. Period마다 Health를 회복한다.
	// Ongoing Tag Requirement로 RecentlyDamaged 보유 시 inhibit되도록 설정해야 한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Regen")
	TSubclassOf<UGameplayEffect> HealthRegenEffect;

	// 피격 시 적용되는 Duration GE. RecentlyDamaged 태그를 부여하며 재피격 시 Duration이 리프레시되도록 설정해야 한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Regen")
	TSubclassOf<UGameplayEffect> CombatTimeoutEffect;

	UFUNCTION(BlueprintNativeEvent)
	void ApplyMaxSpeed(float InMaxSpeed);
	virtual void ApplyMaxSpeed_Implementation(float InMaxSpeed);

	// Debuff.Stunned 태그 변경 콜백. 이동속도 0 처리는 MaxSpeed 어트리뷰트(GE)가 이미 담당하므로,
	// 여기선 bStunned 갱신 + OnStunnedChanged 훅 호출만 한다.
	void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	// 자식 클래스가 스턴 반응을 추가하는 지점(Enemy는 BT Blackboard 미러링, Player는 없음).
	virtual void OnStunnedChanged(bool bNewStunned);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bStunned = false;

private:
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	void OnMaxSpeedChanged(const FOnAttributeChangeData& Data);
	void ApplyCurrentMaxSpeed();
};
