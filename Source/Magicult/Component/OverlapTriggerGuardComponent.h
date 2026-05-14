#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OverlapTriggerGuardComponent.generated.h"

class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct FGuardedOverlapEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Overlap")
	TObjectPtr<UPrimitiveComponent> SourceComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Overlap")
	TObjectPtr<AActor> OtherActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Overlap")
	TObjectPtr<UPrimitiveComponent> OtherComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Overlap")
	int32 OtherBodyIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Overlap")
	bool bFromSweep = false;

	UPROPERTY(BlueprintReadOnly, Category = "Overlap")
	FHitResult SweepResult;

	UPROPERTY(BlueprintReadOnly, Category = "Overlap")
	bool bIsRehit = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuardedOverlap, const FGuardedOverlapEvent&, OverlapEvent);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class MAGICULT_API UOverlapTriggerGuardComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOverlapTriggerGuardComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Overlap Guard")
	FOnGuardedOverlap OnGuardedOverlap;

	/**
	 * true면 같은 대상도 RehitCooldown마다 다시 통과시킨다.
	 * false면 이 컴포넌트 생명주기 동안 같은 Actor는 한 번만 통과한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap Guard")
	bool bCanHitSameTargetAgain = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap Guard", meta = (ClampMin = "0.0"))
	float RehitCooldown = 0.25f;

	/**
	 * true면 이 컴포넌트가 직접 Tick하면서 ActiveOverlap 대상에게 재타격을 발생시킨다.
	 * false면 외부에서 ProcessOverlapRehit를 호출해야 한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap Guard")
	bool bUseComponentTickForRehit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap Guard")
	bool bIgnoreOwner = true;

	/**
	 * 보통 공격자 본체를 넣어두면 자기 자신 타격 방지 가능.
	 */
	UPROPERTY(EditAnywhere, Category = "Overlap Guard")
	TSet<TWeakObjectPtr<AActor>> IgnoredActors;

	/**
	 * 서버에서만 Broadcast하고 싶으면 true.
	 * 클라 예측/디버그까지 필요하면 false.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap Guard")
	bool bRequireAuthority = false;
public:
	UFUNCTION(BlueprintCallable, Category = "Overlap Guard")
	void SetCollisionComponent(UPrimitiveComponent* InCollisionComponent);

	UFUNCTION(BlueprintCallable, Category = "Overlap Guard")
	UPrimitiveComponent* GetCollisionComponent() const { return CollisionComponent; }

	UFUNCTION(BlueprintCallable, Category = "Overlap Guard")
	void SetIgnoredActor(AActor* InIgnoredActor) { IgnoredActors.Add(InIgnoredActor); }

	UFUNCTION(BlueprintCallable, Category = "Overlap Guard")
	void ResetGuard();

	UFUNCTION(BlueprintCallable, Category = "Overlap Guard")
	void ResetActor(AActor* Actor);

	/**
	 * bCanHitSameTargetAgain == true일 때,
	 * 현재 겹쳐 있는 대상들을 검사해서 쿨타임이 지난 대상만 다시 Broadcast한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Overlap Guard")
	void ProcessOverlapRehit(float DeltaTime);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlap Guard")
	TObjectPtr<UPrimitiveComponent> CollisionComponent = nullptr;

	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActorsThisLife;

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, float> NextAllowedHitTime;

	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, FGuardedOverlapEvent> ActiveOverlaps;

protected:
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void HandleEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

protected:
	void BindCollisionComponent();
	void UnbindCollisionComponent();

	bool IsAuthorityAllowed() const;
	bool IsValidImpact(AActor* OtherActor) const;
	bool CanPublishOverlap(AActor* OtherActor);
	void RegisterActiveOverlap(const FGuardedOverlapEvent& Event);
	void BroadcastOverlap(const FGuardedOverlapEvent& Event);
	void CleanupInvalidReferences();
};