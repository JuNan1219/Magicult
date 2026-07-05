// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilitySystemComponent.h"
//#include "Magicult/MagicultAbilityTypes.h"
#include "Magicult/Data/ElementDataTypes.h"
#include "Magicult/Data/CharacterClassInfo.h"
#include "Magicult/Data/OverlapActorSpawnParam.h"
#include "MagicultAbilitySystemLibrary.generated.h"

class UOverlayWidgetController;
class UMagicMenuWidgetController;
class UTagToData;
class UElementInteractionRules;
class UProjectileData;
class AOverlapActor;
class UAttackInstanceConfig;
struct FWidgetContrllerParams;
struct FDamageEffectSpawnParams;
struct FDamageEffectParams;

/**
 * 
 */
UCLASS()
class MAGICULT_API UMagicultAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/*
	* Widget Controller
	*/
	UFUNCTION(BlueprintPure, Category = "MagicultAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AMagicultHUD*& OutMagicultHUD);

	UFUNCTION(BlueprintPure, Category = "MagicultAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "MagicultAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UMagicMenuWidgetController* GetMagicMenuWidgetController(const UObject* WorldContextObject);

	/*
	* Ability System Class Defaults
	*/
	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|CharacterClassDefaults")
	static UMagicInfo* GetMagicInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC, float HealthMultiplier = 1.f);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|DamageEffect")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FAttackApplyParams& AttackApplyParams);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|DamageEffect")
	static FSpellContext MakeSpellContext(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, AActor* SourceActor, AActor* TargetActor, FHitResult HitResult, bool bHasHitResult = false, UObject* DamageCauser = nullptr);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|DamageEffect")
	static bool ApplyDefaultDamageGameplayEffect(const FSpellContext& Context, UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC);

	/*
	* TagToData
	*/

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|TagToData")
	static float GetTagToFloatData(const UObject* WorldContextObject, const FGameplayTag& GameplayTag);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|TagToData")
	static TSubclassOf<AActor> GetTagToClassData(const UObject* WorldContextObject, const FGameplayTag& GameplayTag);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|TagToData")
	static UNiagaraSystem* GetTagToNiagaraData(const UObject* WorldContextObject, const FGameplayTag& GameplayTag);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|TagToData")
	static float GetTagToScalableFloatData(const UObject* WorldContextObject, const FGameplayTag& GameplayTag, int32 Level);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|ElementInteractionRules")
	static bool GetElementReactionRules(const UObject* WorldContextObject, TArray<FElementReactionRule>& OutElementReactionRule);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|ElementInteractionRules")
	static bool GetDebuffRules(const UObject* WorldContextObject, TArray<FDebuffRule>& OutDebuffRule);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|ProjectileData")
	static bool GetElementData(const UObject* WorldContextObject, TArray<FSpellElementSpec>& OutElementData);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|ProjectileData")
	static bool GetAmmoTypeData(const UObject* WorldContextObject, TArray<FSpellProjectileFeatureSpec>& OutAmmoTypeData);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|ProjectileData")
	static bool GetProjectileBaseClass(const UObject* WorldContextObject, TArray<FSpellProjectileClassEntry>& OutProjectileBaseClasses);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|ProjectileData")
	static bool GetModifierConfigs(const UObject* WorldContextObject, TArray<FSpellModifierConfig>& OutModifierConfigs);

	/*
	* TagToData End
	*/
	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|GameplayMechanics")
	static void EnqueueTriggerEvent(const UObject* WorldContextObject, FGameplayTag TriggerTag, FGameplayTagContainer TriggerContextTags, const FSpellContext& SpellContext);

	UFUNCTION(BlueprintPure, Category = "MagicultAbilitySystemLibrary|GameplayMechanics")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);

	UFUNCTION(BlueprintPure, Category = "MagicultAbilitySystemLibrary|GameplayMechanics")
	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors);

	UFUNCTION(BlueprintPure, Category = "MagicultAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);

	UFUNCTION(BlueprintPure, Category = "MagicultAbilitySystemLibrary|GameplayMechanics")
	static FAttackContext GetAttackContextFromEffectContext(FGameplayEffectContextHandle ContextHandle);

	UFUNCTION(BlueprintCallable, Category = "MagicultAbilitySystemLibrary|GameplayMechanics")
	static AOverlapActor* SpawnOverlapActor(UObject* WorldContextObject, const FOverlapActorSpawnParams& SpawnParam);
private:
	static TWeakObjectPtr<UTagToData> CachedTagToData;
	static UTagToData* GetTagToData(const UObject* WorldContextObject);

	static TWeakObjectPtr<UElementInteractionRules> CachedElementInteractionRules;
	static UElementInteractionRules* GetElementInteractionRules(const UObject* WorldContextObject);

	static TWeakObjectPtr<UProjectileData> CachedProjectileData;
	static UProjectileData* GetProjectileData(const UObject* WorldContextObject);
};
