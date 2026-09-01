#pragma once

#include "GameplayTagContainer.h"
#include "SpellActionTypes.h"
#include "TriggerActionSet.h"
#include "ActionSet.h"
#include "ProjectileTypes.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UNiagaraSystem;
class AMagicultProjectile;

/*
* 콜라이더 추상화
*/ 
UENUM(BlueprintType)
enum class ESpellColliderShape : uint8
{
	None,
	Sphere,
	Box,
	Capsule
};


USTRUCT(BlueprintType)
struct FSpellColliderSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collider")
	ESpellColliderShape Shape = ESpellColliderShape::None;

	/** Sphere */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collider",
		meta = (EditCondition = "Shape == ESpellColliderShape::Sphere", EditConditionHides))
	float SphereRadius = 0.f;

	/** Box */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collider",
		meta = (EditCondition = "Shape == ESpellColliderShape::Box", EditConditionHides))
	FVector BoxHalfExtent = FVector::ZeroVector;

	/** Capsule */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collider",
		meta = (EditCondition = "Shape == ESpellColliderShape::Capsule", EditConditionHides))
	float CapsuleRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collider",
		meta = (EditCondition = "Shape == ESpellColliderShape::Capsule", EditConditionHides))
	float CapsuleHalfHeight = 0.f;
};

USTRUCT(BlueprintType)
struct FSpellProjectileClassEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	ESpellColliderShape Shape = ESpellColliderShape::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AMagicultProjectile> ProjectileClass = nullptr;
};

/*
* 콜라이더 추상화 끝
*/

/*
* Element 데이터
*/

USTRUCT(BlueprintType)
struct FSpellShieldFeatureSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Shield")
	FSpellColliderSpec ShieldCollider;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Shield")
	TObjectPtr<UNiagaraSystem> ShieldNiagara = nullptr;
};

USTRUCT(BlueprintType)
struct FSpellElementSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element")
	FGameplayTag ElementTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element")
	FSpellColliderSpec ProjectileCollider;

	// 속성 데미지 비율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element")
	float ElementRatio = 0.f; // 0.0f ~ 1.0f

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element")
	TObjectPtr<UNiagaraSystem> ElementNiagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element")
	FSpellShieldFeatureSpec ShieldSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element")
	TArray<FSpellTriggerActionBinding> HitImpactTriggerAction;
};

/*
* Element 데이터 끝
*/


/*
* AmmoType 데이터
*/

UENUM(BlueprintType)
enum class ESpellProjectileFeatureType : uint8
{
	None,
	FastShot,
	Explosive,
	Pierce,
	Bounce,
	Shield
};

UENUM(BlueprintType)
enum class ESpellCollisionEventPolicy : uint8
{
	None,
	OverlapOnly,
	HitOnly,
	OverlapAndHit,
	Bounce
};

USTRUCT(BlueprintType)
struct FSpellFastShotFeatureSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Fast")
	float AdditionalSpeed = 0.f;
};

USTRUCT(BlueprintType)
struct FSpellPierceFeatureSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Pierce", meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxPierceCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Pierce")
	bool bCanHitSameTargetAgain = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Pierce")
	float RehitCooldown = 0.5f;
};

USTRUCT(BlueprintType)
struct FSpellExplosionFeatureSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Explosion")
	TObjectPtr<UTriggerActionSet> CommonActionSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Explosion")
	TMap<FGameplayTag, TObjectPtr<UTriggerActionSet>> ElementActionSets;
};

USTRUCT(BlueprintType)
struct FSpellBounceFeatureSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Bounce", meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxBounceCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature|Bounce", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Restitution = 1.f;
};

USTRUCT(BlueprintType)
struct FSpellProjectileFeatureSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature")
	ESpellProjectileFeatureType FeatureType = ESpellProjectileFeatureType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature")
	FGameplayTag FeatureTag;

	// 이 feature가 기대하는 충돌 이벤트 방식(예: Pierce는 보통 Hit, Shield는 보통 Overlap).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature")
	ESpellCollisionEventPolicy CollisionEventPolicy = ESpellCollisionEventPolicy::OverlapOnly;

	// feature 고유 trigger-action(예: Pierce는 OnWorldHit/OnPierceDepleted, Bounce는 OnBounceDepleted에 DestroyProjectile 바인딩).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature", meta = (TitleProperty = "Trigger"))
	TArray<FSpellTriggerActionBinding> TriggerActionBindings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature",
		meta = (EditCondition = "FeatureType == ESpellProjectileFeatureType::FastShot", EditConditionHides))
	FSpellFastShotFeatureSpec FastShotSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature",
		meta = (EditCondition = "FeatureType == ESpellProjectileFeatureType::Pierce", EditConditionHides))
	FSpellPierceFeatureSpec PierceSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature",
		meta = (EditCondition = "FeatureType == ESpellProjectileFeatureType::Explosive", EditConditionHides))
	FSpellExplosionFeatureSpec ExplosionSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feature",
		meta = (EditCondition = "FeatureType == ESpellProjectileFeatureType::Bounce", EditConditionHides))
	FSpellBounceFeatureSpec BounceSpec;

};

/*
* AmmoType 데이터 끝
*/

USTRUCT(BlueprintType)
struct FSpellRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	int32 RemainingPierceCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	int32 RemainingBounceCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	int32 RemainingSplitCount = 0;

	void Reset()
	{
		RemainingPierceCount = 0;
		RemainingBounceCount = 0;
		RemainingSplitCount = 0;
	}
};

UENUM(BlueprintType)
enum class ESpellModifierType : uint8
{
	None,
	SizeUp,
	LongLife,
	ZeroGravity,
	Friendly,
	Tracking,
};

USTRUCT(BlueprintType)
struct FTrackingModifierSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier|Tracking")
	float SearchRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier|Tracking")
	float HomingAccelerationMagnitude = 3000.0f;
};

USTRUCT(BlueprintType)
struct FSpellModifierConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	ESpellModifierType ModifierType = ESpellModifierType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	FGameplayTag ModifierTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier",
		meta = (EditCondition = "ModifierType == ESpellModifierType::SizeUp", EditConditionHides))
	float SizeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier",
		meta = (EditCondition = "ModifierType == ESpellModifierType::LongLife", EditConditionHides))
	float LifeSpanMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier",
		meta = (EditCondition = "ModifierType == ESpellModifierType::ZeroGravity", EditConditionHides))
	float GravityScaleOverride = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier",
		meta = (EditCondition = "ModifierType == ESpellModifierType::Tracking", EditConditionHides))
	FTrackingModifierSpec TrackingSpec;
};

USTRUCT(BlueprintType)
struct FResolvedSpellProjectileSpec
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	FGameplayTag ElementTag;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	float ElementRatio = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	float BaseDamage = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	FGameplayTag AmmoTypeTag;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	float InitialSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	FVector LaunchDirection = FVector::ForwardVector;

	// 같은 타겟 재타격 허용 여부
	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	bool bCanHitSameTargetAgain = false;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	float RehitCooldown = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	FSpellColliderSpec ProjectileCollider;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	TSubclassOf<AMagicultProjectile> ProjectileBaseClass;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	TObjectPtr<UNiagaraSystem> ProjectileNiagara = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	ESpellCollisionEventPolicy CollisionEventPolicy = ESpellCollisionEventPolicy::None;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	float Bounciness = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved", meta = (TitleProperty = "BindingTag"))
	TArray<FSpellTriggerActionBinding> TriggerActionBindings;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved|Modifier")
	float DamageModifier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved|Modifier")
	float SizeScale = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved|Modifier")
	float GravityScale = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved|Modifier")
	float LifeSpanMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved|Modifier")
	bool bTargetEnemiesOnly = false;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved|Modifier")
	bool bTracking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved|Modifier")
	float TrackingSearchRadius = 1000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved|Modifier")
	float HomingAccelerationMagnitude = 3000.0f;
};

USTRUCT(BlueprintType)
struct FResolvedSpellProjectileResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	FResolvedSpellProjectileSpec Spec;

	UPROPERTY(BlueprintReadOnly, Category = "Resolved")
	FSpellRuntimeState RuntimeState;
};

