#pragma once

#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "SpellTypes.generated.h"

class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FSpellContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> SourceActor;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> DamageCauser;

	UPROPERTY(BlueprintReadWrite)
	FHitResult HitResult;

	UPROPERTY(BlueprintReadWrite)
	FGameplayEffectSpecHandle DamageSpecHandle;

	UPROPERTY(BlueprintReadWrite)
	bool bHasHitResult = false;
};

USTRUCT(BlueprintType)
struct FSpellTagLevelPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;
};

USTRUCT(BlueprintType)
struct FSpellState
{
	GENERATED_BODY()

	// 단일 선택 유형 태그 (FGameplayTag)
	// (예: 속성, 방향 등 상호 배타적인 하나의 선택)

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Magic Config")
	FGameplayTag DirectionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Config")
	FGameplayTag SpeedTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Config")
	FSpellTagLevelPair ElementTagPair; // 예: Spell.Element.Fire

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Config")
	FSpellTagLevelPair AmmoTypeTagPair;

	// 다중 선택 효과 태그
	// (예: 중첩될 수 있는 부가적인 효과나 옵션)
	// TMap은 표준 프로퍼티 리플리케이션으로 net-serialize가 안 되기 때문에(USTRUCT 멤버로는 미지원)
	// CurrentSpellState 전체를 Replicated로 쓰기 위해 TArray<FSpellTagLevelPair>로 둔다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magic Config")
	TArray<FSpellTagLevelPair> EffectTags;

	void Clear()
	{
		DirectionTag = FGameplayTag();
		SpeedTag = FGameplayTag();
		ElementTagPair = FSpellTagLevelPair();
		AmmoTypeTagPair = FSpellTagLevelPair();

		EffectTags.Empty();
	}

	bool HasEffectTag(FGameplayTag InTag) const
	{
		return EffectTags.ContainsByPredicate([&InTag](const FSpellTagLevelPair& Pair) { return Pair.Tag == InTag; });
	}

	// TMap::Add와 동일한 "있으면 갱신, 없으면 추가" 동작
	void AddEffectTag(const FGameplayTag& InTag, int32 InLevel)
	{
		if (FSpellTagLevelPair* Found = EffectTags.FindByPredicate([&InTag](const FSpellTagLevelPair& Pair) { return Pair.Tag == InTag; }))
		{
			Found->Level = InLevel;
		}
		else
		{
			EffectTags.Add(FSpellTagLevelPair(InTag, InLevel));
		}
	}
};


