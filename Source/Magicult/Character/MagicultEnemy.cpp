// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultEnemy.h"
#include "Components/WidgetComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemComponent.h"
#include "Magicult/AbilitySystem/MagicultAttributeSet.h"
#include "Magicult/AI/MagicultAIController.h"
#include "Magicult/UI/Widget/MagicultUserWidget.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"

AMagicultEnemy::AMagicultEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UMagicultAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	AttributeSet = CreateDefaultSubobject<UMagicultAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	BaseWalkSpeed = 250.f;
}

void AMagicultEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;
	MagicultAIController = Cast<AMagicultAIController>(NewController);
	if (MagicultAIController && MagicultAIController->GetBlackboardComponent() && BehaviorTree)
	{
		MagicultAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	}

	// 매치 시작 후에 possess되는 경우(런타임 스폰 등)는 즉시 BT 시작.
	// 레벨 배치 몬스터는 possess가 먼저라 여기선 시작하지 않고 InitializeForMatch를 기다린다.
	if (bMatchStarted)
	{
		StartBehaviorTree();
	}
}

int32 AMagicultEnemy::GetPlayerLevel_Implementation()
{
	return Level;
}

void AMagicultEnemy::Die()
{
	SetLifeSpan(LifeSpan);
	if (MagicultAIController)
	{
		MagicultAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
		MagicultAIController->StopMovement();

	}
	
	Super::Die();
}

void AMagicultEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AMagicultEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void AMagicultEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	if (MagicultAIController && MagicultAIController->GetBlackboardComponent())
	{
		MagicultAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void AMagicultEnemy::BeginPlay()
{
	Super::BeginPlay();

	InitAbilityActorInfo();
	// 능력 부여(패시브 활성화 포함)는 속성 초기화 이후라야 하므로 InitializeForMatch로 옮긴다.
	// (속성 초기화가 BeginPlay→매치 시작으로 미뤄졌기 때문)

	if (UMagicultUserWidget* MagicultUserWidget = Cast<UMagicultUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		MagicultUserWidget->SetWidgetController(this);
	}

	if (const UMagicultAttributeSet* MagicultAS = Cast<UMagicultAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MagicultAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MagicultAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->RegisterGameplayTagEvent(FMagicultGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,
			&AMagicultEnemy::HitReactTagChanged
		);

		OnHealthChanged.Broadcast(MagicultAS->GetHealth());
		OnMaxHealthChanged.Broadcast(MagicultAS->GetMaxHealth());
	}
}

void AMagicultEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UMagicultAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	// 속성 초기화(체력 인원수 배율 포함)는 매치 시작 시 GameMode 주도로 InitializeForMatch에서 수행한다.
	// 레벨 배치 몬스터의 BeginPlay는 인원수 확정 전(맵 로드 시점)에 돌기 때문에 여기서 초기화하지 않는다.

	Super::InitAbilityActorInfo();
}

void AMagicultEnemy::InitializeForMatch(float HealthMultiplier)
{
	if (!HasAuthority()) return;

	// 속성 초기화(체력 배율) → 능력 부여 순서 보존
	UMagicultAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent, HealthMultiplier);
	UMagicultAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);

	// 속성 초기화 이후 서버에서 리젠 GE 적용
	ApplyHealthRegenEffect();

	bMatchStarted = true;
	StartBehaviorTree();
}

void AMagicultEnemy::StartBehaviorTree()
{
	if (!bMatchStarted) return;
	if (MagicultAIController && BehaviorTree)
	{
		MagicultAIController->RunBehaviorTree(BehaviorTree);
		if (MagicultAIController->GetBlackboardComponent())
		{
			MagicultAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
		}
	}
}
