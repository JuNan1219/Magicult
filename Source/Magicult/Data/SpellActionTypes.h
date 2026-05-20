#pragma once

#include "TriggerTypes.h"
#include "DebuffTypes.h"
#include "SpellActionTypes.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UNiagaraSystem;

// 우선순위 별
UENUM(BlueprintType)
enum class ESpellActionType : uint8
{
	None,
	ConsumePierce,
	Stop,
	ApplyDamage,
	ApplyRadialDamage,
	ApplyElement,
	ApplyDebuff,
	SpawnActor,
	ApplyGameplayEffect,
	ModifyProjectile,
	EmitNiagara,
	PlayCue,
	Knockback,
	DestroyActor,
	Interrupt
};

// 필요한 ActionSpec에 변수로 사용
UENUM(BlueprintType)
enum class ESpellActionTargetPolicy : uint8
{
	None,
	Source,
	Target,
	DamageCauser,
	HitActor,
	HitLocation,
	SocketTransform
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ESpellActionRequireFlags : uint8
{
	None = 0,
	HasSourceASC = 1 << 0,
	HasTargetASC = 1 << 1,
	HasSourceActor = 1 << 2,
	HasTargetActor = 1 << 3,
	HasDamageCauser = 1 << 4,
	HasHitResult = 1 << 5,
};

USTRUCT(BlueprintType)
struct FActionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESpellActionType ActionType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RequiredSourceTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer BlockedSourceTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RequiredTargetTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer BlockedTargetTags;
};

ENUM_CLASS_FLAGS(ESpellActionRequireFlags);

USTRUCT(BlueprintType)
struct FSpellConsumePierceActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|ConsumePierce")
	int32 ConsumePierce = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|ConsumePierce")
	FSpellTriggerSpec ActivateTrigger;
};

USTRUCT(BlueprintType)
struct FSpellDebuffActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag DebuffTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDebuffEffectEntry> EffectSpecs;
};

USTRUCT(BlueprintType)
struct FSpellApplyElementActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Explode")
	FGameplayTag IncomingElement;
};

USTRUCT(BlueprintType)
struct FSpellSpawnActorActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Spawn")
	TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Spawn")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Spawn")
	bool bAttachToHitTarget = false;
};

USTRUCT(BlueprintType)
struct FSpellApplyGEActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|GE")
	TSubclassOf<UGameplayEffect> GameplayEffectClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|GE")
	FGameplayTagContainer TargetFilterTags;
};

USTRUCT(BlueprintType)
struct FSpellEmitNiagaraActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Niagara")
	TObjectPtr<UNiagaraSystem> Niagara = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Niagara")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Niagara")
	ESpellActionTargetPolicy TargetPolicy = ESpellActionTargetPolicy::DamageCauser;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Niagara", meta=(EditCondition = "TargetPolicy == ESpellActionTargetPolicy::SocketTransform", EditConditionHides))
	FGameplayTag SocketTag;
};

USTRUCT(BlueprintType)
struct FSpellPlayCueActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Cue")
	FGameplayTag GameplayCueTag;
};

USTRUCT(BlueprintType)
struct FSpellInterruptActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Interrupt")
	FGameplayTagContainer CancelTags;
};

USTRUCT(BlueprintType)
struct FSpellKnockbackActionSpec
{
	GENERATED_BODY()

	// 넉백을 가할 기준 위치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Knockback")
	ESpellActionTargetPolicy OriginPolicy = ESpellActionTargetPolicy::DamageCauser;

	// 넉백을 받을 대상
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Knockback")
	ESpellActionTargetPolicy TargetPolicy = ESpellActionTargetPolicy::Target;

	// 기본 넉백 힘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Knockback")
	float Strength = 600.f;

	// 위쪽 힘 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Knockback")
	float UpwardStrength = 120.f;

	// true면 Origin -> Target 방향, false면 Source/Hit 방향 기반 전방 밀기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Knockback")
	bool bRadialFromOrigin = false;

	// XY 정규화해서 거리 영향 없이 동일 세기로 밀지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Knockback")
	bool bNormalizeHorizontalDirection = true;

	// physics simulating primitive면 impulse 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Knockback")
	bool bAffectPhysicsBodies = true;

	// 넉백 성공 시 자동으로 발동할 인터럽트 (CancelTags가 비어 있으면 스킵)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Knockback")
	FSpellInterruptActionSpec InterruptSpec;
};

USTRUCT(BlueprintType)
struct FSpellRadialDamageActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|RadialDamage")
	float Radius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|RadialDamage")
	bool bIgnoreOwner = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|RadialDamage")
	FSpellKnockbackActionSpec KnockbackSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|RadialDamage")
	bool bApplyKnockback = false;
};

USTRUCT(BlueprintType)
struct FSpellActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	ESpellActionType ActionType = ESpellActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "/Script/Magicult.ESpellActionRequireFlags"))
	int32 RequiredContextFlags = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	FGameplayTagContainer ActionTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::ConsumePierce", EditConditionHides))
	FSpellConsumePierceActionSpec ConsumePierceSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::ApplyRadialDamage", EditConditionHides))
	FSpellRadialDamageActionSpec RadialDamageSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::ApplyElement", EditConditionHides))
	FSpellApplyElementActionSpec ApplyElementSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::ApplyDebuff", EditConditionHides))
	FSpellDebuffActionSpec DebuffSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::SpawnActor", EditConditionHides))
	FSpellSpawnActorActionSpec SpawnActorSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::ApplyGameplayEffect", EditConditionHides))
	FSpellApplyGEActionSpec ApplyGESpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::EmitNiagara", EditConditionHides))
	FSpellEmitNiagaraActionSpec EmitNiagaraSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::PlayCue", EditConditionHides))
	FSpellPlayCueActionSpec PlayCueSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::Knockback", EditConditionHides))
	FSpellKnockbackActionSpec KnockbackSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::Interrupt", EditConditionHides))
	FSpellInterruptActionSpec InterruptSpec;
};

USTRUCT(BlueprintType)
struct FSpellTriggerActionBinding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TriggerAction")
	FSpellTriggerSpec Trigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TriggerAction")
	TArray<FSpellActionSpec> Actions;
};

USTRUCT(BlueprintType)
struct FSpellActionExecutionRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FSpellActionSpec Action;

	UPROPERTY(BlueprintReadOnly)
	FSpellContext Context;
};

USTRUCT(BlueprintType)
struct FSpellActionExecutionResult
{
	GENERATED_BODY()
	
	// 추가로 발생시킨 트리거 
	UPROPERTY(BlueprintReadOnly)
	TArray<FSpellTriggerEventData> GeneratedTriggerEvents;

	// 추가로 발생시킨 액션
	UPROPERTY(BlueprintReadOnly)
	TArray<FSpellActionExecutionRequest> GeneratedActionRequests;

	UPROPERTY(BlueprintReadOnly)
	bool bStopExecution = false;
};
