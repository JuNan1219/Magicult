#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/Interface/ActionSourceInterface.h"
#include "Magicult/Interface/AttackContextProvider.h"
#include "AttackInstance.generated.h"

class UMagicultGameplayAbility;
struct FGameplayEventData;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttackInstanceFinished, UAttackInstance*);

UENUM(BlueprintType)
enum class EAttackInstanceFinishReason : uint8
{
	None,
	Normal,
	Timeout,
	Cancelled
};

/**
 * 클라이언트에 존재하지 않는 클래스이므로 Emit Niagara 대신 GameplayCue로 이펙트 소리 재생 필요
 */
UCLASS(Blueprintable, BlueprintType)
class MAGICULT_API UAttackInstance : public UObject, public IActionSourceInterface, public IAttackContextProvider
{
	GENERATED_BODY()

public:
	void Initialize(const FSpellContext& InSpellContext, const FAttackSpec& InAttackSpec, bool bOverrideAttackSpec = false);

	void SetAttackSpec(const FAttackSpec& InAttackSpec);

	void SetSocketTag(FGameplayTag InSocketTag);

	virtual void SetInstanceConfig(UAttackInstanceConfig* InConfig);

	void Execute();
	void Finish();
	UFUNCTION(BlueprintCallable)
	void Cancel();

	bool IsActive() const { return bActive; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HandleEventTag(const FGameplayTag& EventTag);

	virtual void HandleEventTag_Implementation(const FGameplayTag& EventTag);

	FOnAttackInstanceFinished OnFinished;
	
	virtual const TArray<FSpellTriggerActionBinding>& GetTriggerActionBindings() const override;

	EAttackInstanceFinishReason GetFinishReason() const { return FinishReason; }
	/*
	* AttackContextProvider Interface 
	*/
	virtual FAttackSpec GetAttackSpec() const override;
	virtual FVector GetActorVelocity() const override;
	/*
	* AttackContextProvider Interface End
	*/
	UFUNCTION(BlueprintCallable, Category = "AttackInstance")
	void SetOwnerActor(AActor* InOwner);

	UFUNCTION(BlueprintPure, Category = "AttackInstance")
	AActor* GetOwnerActor() const;

	UPROPERTY(EditAnywhere,BlueprintReadWrite ,Category = "AttackInstance")
	FGameplayTag SocketTag;
protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnExecuteStarted();
	virtual void OnExecuteStarted_Implementation();

	UFUNCTION(BlueprintNativeEvent)
	void OnFinishedInternal();
	virtual void OnFinishedInternal_Implementation();

	virtual UWorld* GetWorld() const override;

	void StartAutoFinishTimer();
	void ClearAutoFinishTimer();
	void HandleAutoFinishTimeout();
	void FinishInternal(EAttackInstanceFinishReason InReason);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAttackInstanceConfig> AttackConfig;
protected:
	UPROPERTY(BlueprintReadOnly)
	FSpellContext Context;

	// attck 기본값
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAttackSpec AttackSpec;

	UPROPERTY(BlueprintReadOnly)
	bool bActive = false;
	UPROPERTY(BlueprintReadOnly)
	bool bFinishing = false;

	// Blueprint에서 설정하는 바인딩 원본
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category = "TriggerActionBinding")
	TArray<FSpellTriggerActionBinding> TriggerActionBindings;

	UPROPERTY(EditAnywhere, Category = "AttackInstance|Lifetime")
	bool bUseAutoFinish = true;

	UPROPERTY(EditAnywhere, Category = "AttackInstance|Lifetime", meta = (EditCondition = "bUseAutoFinish", ClampMin = "0.0"))
	float AutoFinishDelay = 3.0f;

	FTimerHandle AutoFinishTimerHandle;

	EAttackInstanceFinishReason FinishReason = EAttackInstanceFinishReason::None;

	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
};