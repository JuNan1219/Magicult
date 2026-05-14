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
		MagicultAIController->RunBehaviorTree(BehaviorTree);
		MagicultAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
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
	if (HasAuthority())
	{
		UMagicultAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}

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

	if (HasAuthority())
	{
		InitializeDefaultAttributes();
	}

	Super::InitAbilityActorInfo();
}

void AMagicultEnemy::InitializeDefaultAttributes() const
{
	UMagicultAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}
