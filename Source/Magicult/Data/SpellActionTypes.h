#pragma once

#include "TriggerTypes.h"
#include "DebuffTypes.h"
#include "SpellActionTypes.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UAttackInstance;
class UAttackInstanceConfig;

// 우선순위 별
UENUM(BlueprintType)
enum class ESpellActionType : uint8
{
	None,
	ConsumePierce,
	ApplyDamage,
	ApplyElement,
	ApplyDebuff,
	SpawnActor,
	ApplyGameplayEffect,
	PlayCue,
	Knockback,
	DestroyActor,
	Interrupt,
	SpawnAttackInstance
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

UENUM(BlueprintType)
enum class ESpellCuePlayPolicy : uint8
{
	// ASC 없이 위치 기반으로 재생하고 GameState 멀티캐스트로 전 클라에 전파한다.
	// TargetActor가 GameState라 부착·대상 필터·비월드 카메라셰이크는 쓸 수 없다.
	World,
	// 대상 ASC에서 재생. 대상 몸에 부착되거나 맞은 본인만 흔드는 연출용.
	Target,
	// 시전자 ASC에서 재생.
	Source
};


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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag DebuffTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDebuffEffectEntry> EffectSpecs;

	// 디버프를 받을 대상. 기본값은 Target(피격 대상). 지형 충돌로 자기 자신을 기절시키는 등의
	// 케이스에서는 Source로 지정해 시전자 자신을 대상으로 리다이렉트한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESpellActionTargetPolicy TargetPolicy = ESpellActionTargetPolicy::Target;
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
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Spawn")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Spawn")
	bool bAttachToHitTarget = false;
};

USTRUCT(BlueprintType)
struct FSpellSpawnAttackInstanceActionSpec
{
	GENERATED_BODY()

	// 생성할 논리 공격(AttackInstance) 클래스. 수집·발행 전략은 이 클래스가 정한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|SpawnAttackInstance")
	TSubclassOf<UAttackInstance> AttackInstanceClass;

	// AttackInstance 초기화용 추상 Config(예: URadialDamageConfig).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|SpawnAttackInstance")
	TObjectPtr<UAttackInstanceConfig> InstanceConfig = nullptr;

	// 생성될 AttackInstance의 origin(수집 중심)을 어디서 뽑을지. 발사체 폭발=HitLocation, 자기중심=Source 등.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|SpawnAttackInstance")
	ESpellActionTargetPolicy OriginPolicy = ESpellActionTargetPolicy::HitLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|SpawnAttackInstance",
		meta = (EditCondition = "OriginPolicy == ESpellActionTargetPolicy::SocketTransform", EditConditionHides))
	FGameplayTag SocketTag;

	// true면 스폰을 유발한 부모 DamageCauser의 AttackSpec을 물려받는다(발사체 폭발이 발사체 데미지를 쓰도록).
	// false면 생성된 AttackInstance 자신의 기본 Spec을 쓴다(고정 데미지 노바 등).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|SpawnAttackInstance")
	bool bInheritSpecFromCauser = true;
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
struct FSpellPlayCueActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Cue")
	FGameplayTag GameplayCueTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Cue")
	ESpellCuePlayPolicy PlayPolicy = ESpellCuePlayPolicy::World;
};

USTRUCT(BlueprintType)
struct FSpellInterruptActionSpec
{
	GENERATED_BODY()

	// 이 인터럽트를 식별하는 태그. ImmunityPolicy의 SubjectTag와 매칭되어 룰 단위로 면역을 걸 수 있다 (예: Interrupt.AttackCancel).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action|Interrupt")
	FGameplayTag InterruptTag;

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
};

USTRUCT(BlueprintType)
struct FSpellActionSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action")
	ESpellActionType ActionType = ESpellActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::ConsumePierce", EditConditionHides))
	FSpellConsumePierceActionSpec ConsumePierceSpec;

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
		meta = (EditCondition = "ActionType == ESpellActionType::PlayCue", EditConditionHides))
	FSpellPlayCueActionSpec PlayCueSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::Knockback", EditConditionHides))
	FSpellKnockbackActionSpec KnockbackSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::Interrupt", EditConditionHides))
	FSpellInterruptActionSpec InterruptSpec;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action",
		meta = (EditCondition = "ActionType == ESpellActionType::SpawnAttackInstance", EditConditionHides))
	FSpellSpawnAttackInstanceActionSpec SpawnAttackInstanceSpec;
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

	// 액션 체인 깊이. 트리거에서 직접 만든 요청=0, 파생 요청=부모+1.
	// ExecuteRequest가 이 값으로 같은 실행 배치 안의 순환(원소 반응 등)을 차단한다.
	int32 ChainDepth = 0;
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

	// 액션이 실제로 성공적으로 적용됐는지 여부
	// true  = 정상 실행 완료
	// false = 면역·유효성 실패 등으로 적용되지 않음
	UPROPERTY(BlueprintReadOnly)
	bool bApplied = true;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag ResultTag;

	// 이 액션이 실제로 실행됐는지(bApplied와 별개). false = 이 머신에 실행 권한이 없어
	// 조기 스킵됨(예: 클라이언트가 권위 액션을 건너뜀). 프레임 액션 예산 집계에서 제외 대상.
	UPROPERTY(BlueprintReadOnly)
	bool bDidWork = true;

	FSpellActionExecutionResult() = default;

	// 성공 여부를 직접 지정하는 생성자. 면역·조기 종료 시 false를 전달한다.
	explicit FSpellActionExecutionResult(bool bWasApplied)
		: bApplied(bWasApplied)
	{}
};
