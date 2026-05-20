// Fill out your copyright notice in the Description page of Project Settings.


#include "ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "Magicult/MagicultAbilityTypes.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "Magicult/MagicultGameplayTags.h"

struct MagicultDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(FlameAffinity)
	DECLARE_ATTRIBUTE_CAPTUREDEF(IceAffinity)
	DECLARE_ATTRIBUTE_CAPTUREDEF(RockAffinity)
	DECLARE_ATTRIBUTE_CAPTUREDEF(WindAffinity)
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningAffinity)

	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageModifier)

	DECLARE_ATTRIBUTE_CAPTUREDEF(FlameResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(IceResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(RockResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(WindResistance)
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance)

	
	MagicultDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, FlameAffinity, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, IceAffinity, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, RockAffinity, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, WindAffinity, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, LightningAffinity, Source, true);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, DamageModifier, Source, true);
									 
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, FlameResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, IceResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, RockResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, WindResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMagicultAttributeSet, LightningResistance, Target, false);
	}
};

static const MagicultDamageStatics& DamageStatics()
{
	static MagicultDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().FlameAffinityDef);
	RelevantAttributesToCapture.Add(DamageStatics().IceAffinityDef);
	RelevantAttributesToCapture.Add(DamageStatics().RockAffinityDef);
	RelevantAttributesToCapture.Add(DamageStatics().WindAffinityDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningAffinityDef);

	RelevantAttributesToCapture.Add(DamageStatics().DamageModifierDef);

	RelevantAttributesToCapture.Add(DamageStatics().FlameResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().IceResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().RockResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().WindResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	const FMagicultGameplayTags& Tags = FMagicultGameplayTags::Get();

	TagsToCaptureDefs.Add(Tags.Attributes_Affinity_FireAffinity, DamageStatics().FlameAffinityDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Affinity_IceAffinity, DamageStatics().IceAffinityDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Affinity_EarthAffinity, DamageStatics().RockAffinityDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Affinity_WindAffinity, DamageStatics().WindAffinityDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Affinity_LightningAffinity, DamageStatics().LightningAffinityDef);

	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_FireResistance, DamageStatics().FlameResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_IceResistance, DamageStatics().IceResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_EarthResistance, DamageStatics().RockResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_WindResistance, DamageStatics().WindResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_LightningResistance, DamageStatics().LightningResistanceDef);

	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Damage = 0.f;
	if (FMagicultGameplayEffectContext* MagicultEffectContext = static_cast<FMagicultGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		float ElementDamage = MagicultEffectContext->AttackContext.BaseDamage * MagicultEffectContext->AttackContext.ElementRatio;
		float PhysicalDamage = MagicultEffectContext->AttackContext.BaseDamage * (1.f - MagicultEffectContext->AttackContext.ElementRatio);

		// 폭발이 아닌 대지속성은 속도에 따른 데미지 모디파이어 적용
		if (MagicultEffectContext->AttackContext.bUseVelocityDamage)
		{
			FVector DamageCauserVelocity = MagicultEffectContext->AttackContext.DamageCauserVelocity;
			FVector TargetVelocity = TargetAvatar ? TargetAvatar->GetVelocity() : FVector::ZeroVector;

			Damage += CalcImpactDamage(DamageCauserVelocity, TargetVelocity, PhysicalDamage, 1000.f);
		}
		else
		{
			Damage += PhysicalDamage;
		}

		FGameplayTag ElementTag = MagicultEffectContext->AttackContext.ElementType;
		if (Tags.ElementsToAffinity.Contains(ElementTag) && Tags.ElementsToResistances.Contains(ElementTag))
		{
			const FGameplayTag AffinityTag = Tags.ElementsToAffinity[ElementTag];
			const FGameplayTag ResistanceTag = Tags.ElementsToResistances[ElementTag];

			checkf(TagsToCaptureDefs.Contains(AffinityTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *AffinityTag.ToString());
			checkf(TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
			const FGameplayEffectAttributeCaptureDefinition CaptureAffinityDef = TagsToCaptureDefs[AffinityTag];
			const FGameplayEffectAttributeCaptureDefinition CaptureResistanceDef = TagsToCaptureDefs[ResistanceTag];

			// 속성 친화, 저항 계산
			if (ElementDamage > 0.f)
			{
				float ElementModifier = 0.f;
				float Affinity = 0.f;
				float Resistance = 0.f;
				ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureAffinityDef, EvaluationParameters, Affinity);
				ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureResistanceDef, EvaluationParameters, Resistance);

				// 클램프 한계를 두지 않아 힐로 받을 수도 있음
				ElementModifier = (1 + 0.1f * (Affinity - Resistance));
				ElementDamage *= ElementModifier;
				Damage += ElementDamage;
			}
		}

		Damage *= MagicultEffectContext->AttackContext.DamageModifier;
	}

	const FGameplayModifierEvaluatedData EvaluatedData(UMagicultAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}

float UExecCalc_Damage::CalcImpactDamage(const FVector& VelocityA, const FVector& VelocityB, float BaseDamage, float SpeedDivisor, float MinMultiplier, float MaxMultiplier) const
{
	if (SpeedDivisor <= KINDA_SMALL_NUMBER)
	{
		return BaseDamage;
	}

	// 상대속도
	const FVector RelativeVelocity = VelocityA - VelocityB;

	// 상대속도의 크기
	const float RelativeSpeed = RelativeVelocity.Size();

	// 속도를 기준값으로 나눠 배율 계산
	float DamageMultiplier = RelativeSpeed / SpeedDivisor;

	// 너무 작거나 너무 커지는 것 방지
	DamageMultiplier = FMath::Clamp(DamageMultiplier, MinMultiplier, MaxMultiplier);

	// 최종 데미지
	return BaseDamage * DamageMultiplier;
}
