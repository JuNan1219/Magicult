#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CollisionGuardComponent.generated.h"

class UPrimitiveComponent;

UENUM(BlueprintType)
enum class EGuardedCollisionType : uint8
{
	Overlap,
	Hit,
};

USTRUCT(BlueprintType)
struct FGuardedCollisionEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EGuardedCollisionType CollisionType = EGuardedCollisionType::Overlap;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPrimitiveComponent> SourceComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> OtherActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPrimitiveComponent> OtherComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FHitResult HitResult;

	UPROPERTY(BlueprintReadOnly)
	bool bIsRehit = false;

	UPROPERTY(BlueprintReadOnly)
	bool bFromSweep = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuardedCollision, const FGuardedCollisionEvent&, CollisionEvent);

/**
 * Overlap + Hit 이벤트를 모두 처리하는 통합 충돌 가드 컴포넌트.
 * 두 이벤트 타입 모두 같은 dedup 테이블을 공유한다.
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class MAGICULT_API UCollisionGuardComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCollisionGuardComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Collision Guard")
	FOnGuardedCollision OnGuardedCollision;

	/** true면 쿨다운 후 같은 대상을 다시 통과시킨다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Guard")
	bool bCanHitSameTargetAgain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Guard", meta = (ClampMin = "0.0"))
	float RehitCooldown = 0.25f;

	/** true면 Tick에서 Overlap 재타격을 자동 처리한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Guard")
	bool bUseComponentTickForRehit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Guard")
	bool bIgnoreOwner = true;

	UPROPERTY(EditAnywhere, Category = "Collision Guard")
	TSet<TWeakObjectPtr<AActor>> IgnoredActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Guard")
	bool bRequireAuthority = false;

	/** Overlap 이벤트를 수신할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Guard")
	bool bHandleOverlap = true;

	/** Hit 이벤트를 수신할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Guard")
	bool bHandleHit = true;

public:
	UFUNCTION(BlueprintCallable, Category = "Collision Guard")
	void SetCollisionComponent(UPrimitiveComponent* InCollisionComponent);

	UFUNCTION(BlueprintCallable, Category = "Collision Guard")
	UPrimitiveComponent* GetCollisionComponent() const { return CollisionComponent; }

	UFUNCTION(BlueprintCallable, Category = "Collision Guard")
	void SetIgnoredActor(AActor* InIgnoredActor) { IgnoredActors.Add(InIgnoredActor); }

	UFUNCTION(BlueprintCallable, Category = "Collision Guard")
	void ResetGuard();

	UFUNCTION(BlueprintCallable, Category = "Collision Guard")
	void ResetActor(AActor* Actor);

	/** Overlap 재타격 수동 처리 (bUseComponentTickForRehit == false일 때 외부 호출용) */
	UFUNCTION(BlueprintCallable, Category = "Collision Guard")
	void ProcessOverlapRehit(float DeltaTime);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Guard")
	TObjectPtr<UPrimitiveComponent> CollisionComponent = nullptr;

	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActorsThisLife;

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, float> NextAllowedHitTime;

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, FGuardedCollisionEvent> ActiveOverlaps;

protected:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	void BindCollisionComponent();
	void UnbindCollisionComponent();

	bool IsAuthorityAllowed() const;
	bool IsValidImpact(AActor* OtherActor) const;
	bool CanPublish(AActor* OtherActor);
	void Broadcast(const FGuardedCollisionEvent& Event);
	void CleanupInvalidReferences();
};
