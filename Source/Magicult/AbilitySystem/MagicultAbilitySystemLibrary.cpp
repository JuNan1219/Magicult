// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultAbilitySystemLibrary.h"
#include "Magicult/AbilitySystem/Abilities/MagicultPassiveAbility.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "Magicult/Player/MagicultPlayerState.h"
#include "Magicult/UI/HUD/MagicultHUD.h"
#include "Magicult/UI/WidgetController/MagicultWidgetController.h"
#include "Magicult/Game/MagicultGameModeBase.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/Data/TagToData.h"
#include "Magicult/Data/ElementInteractionRules.h"
#include "Magicult/Data/ProjectileData.h"
#include "Magicult/Data/SpellTypes.h"
#include "Magicult/MagicultAbilityTypes.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/Actor/MagicultProjectile.h"
#include "Magicult/Subsystem/CombatSubsystem.h"
#include "Magicult/Actor/OverlapActor.h"
#include "Magicult/Data/OverlapActorSpawnParam.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "GameplayEffect.h"

TWeakObjectPtr<UTagToData> UMagicultAbilitySystemLibrary::CachedTagToData = nullptr;
TWeakObjectPtr<UElementInteractionRules> UMagicultAbilitySystemLibrary::CachedElementInteractionRules = nullptr;
TWeakObjectPtr<UProjectileData> UMagicultAbilitySystemLibrary::CachedProjectileData = nullptr;

bool UMagicultAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AMagicultHUD*& OutMagicultHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutMagicultHUD = Cast<AMagicultHUD>(PC->GetHUD());
		if (OutMagicultHUD)
		{
			AMagicultPlayerState* PS = PC->GetPlayerState<AMagicultPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();

			OutWCParams.AttributeSet = AS;
			OutWCParams.AbilitySystemComponent = ASC;
			OutWCParams.PlayerState = PS;
			OutWCParams.PlayerController = PC;
			return true;
		}
	}
	return false;
}

UOverlayWidgetController* UMagicultAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AMagicultHUD* MagicultHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, MagicultHUD))
	{
		return MagicultHUD->GetOverlayWidgetController(WCParams);
	}
	return nullptr;
}

UMagicMenuWidgetController* UMagicultAbilitySystemLibrary::GetMagicMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AMagicultHUD* MagicultHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, MagicultHUD))
	{
		return MagicultHUD->GetMagicMenuWidgetController(WCParams);
	}
	return nullptr;
}

UMagicInfo* UMagicultAbilitySystemLibrary::GetMagicInfo(const UObject* WorldContextObject)
{
	const AMagicultGameModeBase* MagicultGameMode = Cast<AMagicultGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (MagicultGameMode == nullptr) return nullptr;
	return MagicultGameMode->MagicInfo;

}

UCharacterClassInfo* UMagicultAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const AMagicultGameModeBase* MagicultGameMode = Cast<AMagicultGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (MagicultGameMode == nullptr) return nullptr;
	return MagicultGameMode->CharacterClassInfo;
}

void UMagicultAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC, float HealthMultiplier)
{
	AActor* AvatarActor = ASC->GetAvatarActor();

	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(AvatarActor);

	auto ApplyEffectToSelf = [ASC, Level, &EffectContextHandle](TSubclassOf<UGameplayEffect> EffectClass)
		{
			if (!EffectClass) return;
			const FGameplayEffectSpecHandle SpecHandle =
				ASC->MakeOutgoingSpec(EffectClass, Level, EffectContextHandle);
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		};

	// 개별
	ApplyEffectToSelf(ClassDefaultInfo.PrimaryAttributes);
	// 공통
	ApplyEffectToSelf(CharacterClassInfo->SecondaryAttributes);
	ApplyEffectToSelf(CharacterClassInfo->VitalAttributes);

	// 인원수 체력 배율. Infinite Modifier GE로 배율을 걸면 CurrentValue만 바뀌고 BaseValue는 그대로라,
	// EGameplayEffectReplicationMode::Minimal인 Enemy ASC에서는 ActiveGameplayEffects 자체가
	// 클라에 복제되지 않아 클라 로컬 Aggregator가 배율 없는 BaseValue로 CurrentValue를 재계산해버린다
	// (MaxHealth만 서버-클라 불일치가 나던 원인). SetNumericAttributeBase로 BaseValue 자체에
	// 스케일된 최종값을 박아 Health와 동일한 방식으로 복제되게 한다.
	// 배율이 1이거나 비정상(0 이하)이면 스킵 → 플레이어 등 기본 호출자는 영향 없음.
	if (HealthMultiplier > 0.f && !FMath::IsNearlyEqual(HealthMultiplier, 1.f))
	{
		const float BaseMaxHealth = ASC->GetNumericAttribute(UMagicultAttributeSet::GetMaxHealthAttribute());
		const float NewMaxHealth = BaseMaxHealth * HealthMultiplier;
		ASC->SetNumericAttributeBase(UMagicultAttributeSet::GetMaxHealthAttribute(), NewMaxHealth);

		// 스케일된 최종 MaxHealth로 Health를 풀로 채운다.
		ASC->SetNumericAttributeBase(UMagicultAttributeSet::GetHealthAttribute(), NewMaxHealth);
	}
}

void UMagicultAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
			ASC->GiveAbility(AbilitySpec);
		}
	}

	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (Cast<UMagicultPassiveAbility>(AbilitySpec.Ability))
		{
			FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
			ASC->TryActivateAbility(Handle);
		}
	}
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (Cast<UMagicultPassiveAbility>(AbilitySpec.Ability))
		{
			FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
			ASC->TryActivateAbility(Handle);
		}
	}
}

FGameplayEffectContextHandle UMagicultAbilitySystemLibrary::ApplyDamageEffect(const FAttackApplyParams& AttackApplyParams)
{
	const FMagicultGameplayTags& GameplayTags = FMagicultGameplayTags::Get();
	const AActor* SourceAvatarActor = AttackApplyParams.SourceAbilitySystemComponent->GetAvatarActor();

	FGameplayEffectContextHandle EffectContexthandle = AttackApplyParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle SpecHandle = AttackApplyParams.SourceAbilitySystemComponent->MakeOutgoingSpec(AttackApplyParams.DamageGameplayEffectClass, 1, EffectContexthandle);

	AttackApplyParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	return EffectContexthandle;
}

FSpellContext UMagicultAbilitySystemLibrary::MakeSpellContext(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, AActor* SourceActor, AActor* TargetActor, FHitResult HitResult, bool bHasHitResult, UObject* DamageCauser)
{
	FSpellContext ResultSpellContext;
	ResultSpellContext.SourceAbilitySystemComponent = SourceASC;
	ResultSpellContext.TargetAbilitySystemComponent = TargetASC;
	ResultSpellContext.SourceActor = SourceActor;
	ResultSpellContext.TargetActor = TargetActor;
	ResultSpellContext.HitResult = HitResult;
	ResultSpellContext.bHasHitResult = bHasHitResult;

	ResultSpellContext.DamageCauser = DamageCauser;

	return ResultSpellContext;
}
static void FillDamageEffectContext(const FSpellContext& Context, FGameplayEffectContextHandle& EffectContext)
{
	EffectContext.AddSourceObject(
		Context.DamageCauser.Get()
		? Context.DamageCauser.Get()
		: static_cast<UObject*>(Context.SourceActor.Get())
	);

	if (Context.SourceActor.IsValid())
	{
		AActor* CauserActor = Cast<AActor>(Context.DamageCauser.Get());
		EffectContext.AddInstigator(
			Context.SourceActor.Get(),
			CauserActor ? CauserActor : Context.SourceActor.Get());
	}

	if (Context.bHasHitResult)
	{
		EffectContext.AddHitResult(Context.HitResult, true);
	}

	if (FMagicultGameplayEffectContext* MagicultContext = static_cast<FMagicultGameplayEffectContext*>(EffectContext.Get()))
	{
		if (const IAttackContextProvider* AttackContextProvider = Cast<IAttackContextProvider>(Context.DamageCauser.Get()))
		{
			const FAttackSpec& AttackSpec = AttackContextProvider->GetAttackSpec();
			MagicultContext->AttackContext.ElementType = AttackSpec.ElementTag;
			MagicultContext->AttackContext.ElementRatio = AttackSpec.ElementRatio;
			MagicultContext->AttackContext.BaseDamage = AttackSpec.BaseDamage;
			MagicultContext->AttackContext.bUseVelocityDamage = AttackSpec.bUseVelocityDamage;
			MagicultContext->AttackContext.DamageModifier = AttackSpec.DamageModifier;
			MagicultContext->AttackContext.DamageCauserVelocity = AttackContextProvider->GetActorVelocity();
		}
	}
}
bool UMagicultAbilitySystemLibrary::ApplyDefaultDamageGameplayEffect(const FSpellContext& Context, UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC)
{
	if (!TargetASC)
	{
		return false;
	}

	FGameplayEffectSpec* Spec = nullptr;
	FGameplayEffectSpecHandle LocalSpecHandle;

	if (Context.DamageSpecHandle.IsValid())
	{
		Spec = Context.DamageSpecHandle.Data.Get();
	}
	else
	{
		// SourceASC가 없을 수 있으므로 Spec 생성용 ASC를 따로 결정
		UAbilitySystemComponent* SpecOwnerASC = SourceASC ? SourceASC : TargetASC;

		if (!SpecOwnerASC)
		{
			return false;
		}

		UObject* WorldContextObject = nullptr;

		if (Context.DamageCauser.IsValid())
		{
			WorldContextObject = Context.DamageCauser.Get();
		}
		else
		{
			WorldContextObject = SpecOwnerASC;
		}

		AMagicultGameModeBase* MagicultGameMode =
			Cast<AMagicultGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));

		if (!MagicultGameMode || !MagicultGameMode->DefaultDamageGameplayEffect)
		{
			return false;
		}

		FGameplayEffectContextHandle EffectContext = SpecOwnerASC->MakeEffectContext();

		LocalSpecHandle = SpecOwnerASC->MakeOutgoingSpec(
			MagicultGameMode->DefaultDamageGameplayEffect,
			1.f,
			EffectContext
		);

		if (!LocalSpecHandle.IsValid())
		{
			return false;
		}

		Spec = LocalSpecHandle.Data.Get();
	}

	if (!Spec)
	{
		return false;
	}

	FGameplayEffectContextHandle EffectContext = Spec->GetContext();
	FillDamageEffectContext(Context, EffectContext);

	TargetASC->ApplyGameplayEffectSpecToSelf(*Spec);
	return true;
}

UTagToData* UMagicultAbilitySystemLibrary::GetTagToData(const UObject* WorldContextObject)
{
	if (UGameplayStatics::GetGameMode(WorldContextObject) == nullptr) return nullptr;

	if (!CachedTagToData.IsValid())
	{
		if (AMagicultGameModeBase* MagicultGamemode = Cast<AMagicultGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject)))
		{
			CachedTagToData = MagicultGamemode->TagToData;
		}
	}
	return CachedTagToData.Get();
}

UElementInteractionRules* UMagicultAbilitySystemLibrary::GetElementInteractionRules(const UObject* WorldContextObject)
{
	if (UGameplayStatics::GetGameMode(WorldContextObject) == nullptr) return nullptr;

	if (!CachedElementInteractionRules.IsValid())
	{
		if (AMagicultGameModeBase* MagicultGamemode = Cast<AMagicultGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject)))
		{
			CachedElementInteractionRules = MagicultGamemode->ElementInteractionRules;
		}
	}
	return CachedElementInteractionRules.Get();
}

UProjectileData* UMagicultAbilitySystemLibrary::GetProjectileData(const UObject* WorldContextObject)
{
	if (UGameplayStatics::GetGameMode(WorldContextObject) == nullptr) return nullptr;

	if (!CachedProjectileData.IsValid())
	{
		if (AMagicultGameModeBase* MagicultGamemode = Cast<AMagicultGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject)))
		{
			CachedProjectileData = MagicultGamemode->ProjectileData;
		}
	}
	return CachedProjectileData.Get();
}

float UMagicultAbilitySystemLibrary::GetTagToFloatData(const UObject* WorldContextObject, const FGameplayTag& GameplayTag)
{
	UTagToData* TagToData = GetTagToData(WorldContextObject);

	if (!IsValid(TagToData)) return -1.0f;
	auto* Value = TagToData->FloatValues.Find(GameplayTag);
	return Value == nullptr ? -1.0f : *Value;
}

TSubclassOf<AActor> UMagicultAbilitySystemLibrary::GetTagToClassData(const UObject* WorldContextObject, const FGameplayTag& GameplayTag)
{
	UTagToData* TagToData = GetTagToData(WorldContextObject);

	if (!IsValid(TagToData)) return nullptr;
	auto* Value = TagToData->ClassValues.Find(GameplayTag);
	return Value ? *Value : nullptr;
}

UNiagaraSystem* UMagicultAbilitySystemLibrary::GetTagToNiagaraData(const UObject* WorldContextObject, const FGameplayTag& GameplayTag)
{
	UTagToData* TagToData = GetTagToData(WorldContextObject);

	if (!IsValid(TagToData)) return nullptr;
	auto* Value = TagToData->NiagaraValues.Find(GameplayTag);
	return Value ? *Value : nullptr;
}

float UMagicultAbilitySystemLibrary::GetTagToScalableFloatData(const UObject* WorldContextObject, const FGameplayTag& GameplayTag, int32 Level)
{
	UTagToData* TagToData = GetTagToData(WorldContextObject);

	if (!IsValid(TagToData)) return -1.f;
	auto* ScalableFloat = TagToData->ScalableFloatValues.Find(GameplayTag);

	return ScalableFloat ? ScalableFloat->GetValueAtLevel(Level) : -1.f;
}

bool UMagicultAbilitySystemLibrary::GetElementReactionRules(const UObject* WorldContextObject, TArray<FElementReactionRule>& OutElementReactionRule)
{
	OutElementReactionRule.Reset();

	const UElementInteractionRules* ElementInteractionRules = GetElementInteractionRules(WorldContextObject);
	if (!IsValid(ElementInteractionRules))
	{
		return false;
	}

	OutElementReactionRule = ElementInteractionRules->ElementReactionRules;
	return true;
}

bool UMagicultAbilitySystemLibrary::GetDebuffRules(const UObject* WorldContextObject, TArray<FDebuffRule>& OutDebuffRule)
{
	OutDebuffRule.Reset();

	const UElementInteractionRules* ElementInteractionRules = GetElementInteractionRules(WorldContextObject);
	if (!IsValid(ElementInteractionRules))
	{
		return false;
	}

	OutDebuffRule = ElementInteractionRules->DebuffRules;
	return true;
}

bool UMagicultAbilitySystemLibrary::GetElementData(const UObject* WorldContextObject, TArray<FSpellElementSpec>& OutElementData)
{
	OutElementData.Reset();

	const UProjectileData* ProjectileData = GetProjectileData(WorldContextObject);
	if (!IsValid(ProjectileData))
	{
		return false;
	}

	OutElementData = ProjectileData->ElementData;
	return true;
}

bool UMagicultAbilitySystemLibrary::GetAmmoTypeData(const UObject* WorldContextObject, TArray<FSpellProjectileFeatureSpec>& OutAmmoTypeData)
{
	OutAmmoTypeData.Reset();

	const UProjectileData* ProjectileData = GetProjectileData(WorldContextObject);
	if (!IsValid(ProjectileData))
	{
		return false;
	}

	OutAmmoTypeData = ProjectileData->AmmoTypeData;
	return true;
}

bool UMagicultAbilitySystemLibrary::GetProjectileBaseClass(const UObject* WorldContextObject, TArray<FSpellProjectileClassEntry>& OutProjectileBaseClasses)
{
	OutProjectileBaseClasses.Reset();

	const UProjectileData* ProjectileData = GetProjectileData(WorldContextObject);
	if (!IsValid(ProjectileData))
	{
		return false;
	}

	OutProjectileBaseClasses = ProjectileData->ProjectileBaseClasses;
	return true;
}

bool UMagicultAbilitySystemLibrary::GetModifierConfigs(const UObject* WorldContextObject, TArray<FSpellModifierConfig>& OutModifierConfigs)
{
	OutModifierConfigs.Reset();

	const UProjectileData* ProjectileData = GetProjectileData(WorldContextObject);
	if (!IsValid(ProjectileData))
	{
		return false;
	}

	OutModifierConfigs = ProjectileData->ModifierConfigs;
	return true;
}

void UMagicultAbilitySystemLibrary::EnqueueTriggerEvent(const UObject* WorldContextObject, FGameplayTag TriggerTag, FGameplayTagContainer TriggerContextTags, const FSpellContext& SpellContext)
{
	if (!IsValid(WorldContextObject))
	{
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return;
	}

	UCombatSubsystem* CombatSubsystem = World->GetSubsystem<UCombatSubsystem>();
	if (!CombatSubsystem)
	{
		return;
	}

	FSpellTriggerEventData NewTrigger;
	NewTrigger.TriggerTag = TriggerTag;
	NewTrigger.TriggerContextTags = TriggerContextTags;
	NewTrigger.SpellContext = SpellContext;

	CombatSubsystem->Enqueue(NewTrigger);
}

TArray<FRotator> UMagicultAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;

	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumRotators > 1)
	{
		const float DeltaSpread = Spread / (NumRotators - 1);
		for (int32 i = 0; i < NumRotators; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}
	return Rotators;
}

TArray<FVector> UMagicultAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;

	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumVectors > 1)
	{
		const float DeltaSpread = Spread / (NumVectors - 1);
		for (int32 i = 0; i < NumVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Vectors.Add(Direction);
		}
	}
	else
	{
		Vectors.Add(Forward);
	}
	return Vectors;
}

bool UMagicultAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	if (!FirstActor || !SecondActor) return true;

	// 파벌 태그가 같으면 아군. 한쪽이라도 ICombatInterface가 없거나 태그가 비어 있으면 적으로 간주한다.
	if (!FirstActor->Implements<UCombatInterface>() || !SecondActor->Implements<UCombatInterface>())
	{
		return true;
	}

	const FGameplayTag FirstTeam = ICombatInterface::Execute_GetTeamTag(FirstActor);
	const FGameplayTag SecondTeam = ICombatInterface::Execute_GetTeamTag(SecondActor);

	if (!FirstTeam.IsValid() || !SecondTeam.IsValid())
	{
		return true;
	}

	return FirstTeam != SecondTeam;
}

FAttackContext UMagicultAbilitySystemLibrary::GetAttackContextFromEffectContext(FGameplayEffectContextHandle ContextHandle)
{
	if (FMagicultGameplayEffectContext* MagicultGameplayEffectContext = static_cast<FMagicultGameplayEffectContext*>(ContextHandle.Get()))
	{
		return MagicultGameplayEffectContext->AttackContext;
	}

	return FAttackContext();
}

AOverlapActor* UMagicultAbilitySystemLibrary::SpawnOverlapActor(UObject* WorldContextObject, const FOverlapActorSpawnParams& SpawnParam)
{
	if (!WorldContextObject || !SpawnParam.OverlapActorClass)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.Owner = SpawnParam.Owner;
	Params.Instigator = SpawnParam.Instigator;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AOverlapActor* Actor =
		World->SpawnActor<AOverlapActor>(
			SpawnParam.OverlapActorClass,
			SpawnParam.SpawnTransform,
			Params
		);

	if (!Actor)
	{
		return nullptr;
	}

	Actor->InitializeWithConfig(SpawnParam.Context, SpawnParam.CollisionConfig, SpawnParam.ParentTriggerActionBindings, SpawnParam.AttackSpec, SpawnParam.bOverrideAttackSpec);

	return Actor;
}
