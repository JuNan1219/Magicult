// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicultEnemy.h"
#include "Components/WidgetComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
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

	// (A) 넉백 등 급격한 이동이 클라 SimulatedProxy에 자주 코렉션되도록 업데이트 빈도 상향.
	// 플레이어(PlayerState)와 동일하게 100으로 맞춰 스냅샷 간격을 줄인다.
	NetUpdateFrequency = 100.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	AttributeSet = CreateDefaultSubobject<UMagicultAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
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
	// 트리거로 켜야 하는 보스는 여기서도 자동 시작하지 않고 ActivateCombat()을 기다린다.
	if (bMatchStarted && !bWaitForActivationTrigger)
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

		// 사망 시 BT 로직을 완전히 정지한다. 시체 상태로 패턴 선택/이동/공격이 계속 도는 것을 막는다.
		// (LifeSpan으로 파괴되기 전까지 트리가 Combat을 계속 틱하지 않도록)
		if (UBrainComponent* Brain = MagicultAIController->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Dead"));
		}
	}

	// 래그돌/파괴 전에 드랍을 스폰한다(서버 권한에서만).
	SpawnLoot();

	Super::Die();

	OnDeath.Broadcast(this);
}

void AMagicultEnemy::StopCombat()
{
	if (!HasAuthority()) return;

	if (MagicultAIController)
	{
		MagicultAIController->StopMovement();

		if (UBrainComponent* Brain = MagicultAIController->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("CombatStopped"));
		}
	}

	// 트리가 멈춰도 이미 활성화된 공격 GA는 계속 도므로 같이 끊는다.
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}
}

void AMagicultEnemy::SpawnLoot()
{
	// 오버랩/사망 경로가 클라에서도 탈 수 있으므로 권한을 명시적으로 가드한다.
	// 스폰 액터는 bReplicates=true라 서버 스폰만으로 클라에 복제된다(중복 스폰 방지).
	if (!HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector BaseLocation = GetActorLocation();

	for (const FMagicultLootDrop& Drop : LootDrops)
	{
		if (!Drop.ItemClass) continue;

		for (int32 i = 0; i < Drop.MaxCount; ++i)
		{
			// [0,100) 범위 판정. DropChance=100이면 항상, 0이면 절대 스폰 안 됨
			// (Aura의 FRandRange(1,100) off-by-one을 피한다).
			if (FMath::FRandRange(0.f, 100.f) >= Drop.DropChance) continue;

			// 여러 개가 한 점에 겹치지 않도록 XY로 살짝 흩뿌리고 살짝 띄운다.
			const FVector Offset(FMath::RandRange(-60.f, 60.f), FMath::RandRange(-60.f, 60.f), 40.f);
			const FTransform SpawnTransform(FRotator::ZeroRotator, BaseLocation + Offset);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			World->SpawnActor<AActor>(Drop.ItemClass, SpawnTransform, Params);
		}
	}
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
	if (MagicultAIController && MagicultAIController->GetBlackboardComponent())
	{
		MagicultAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
		if (bHitReacting)
		{
			MagicultAIController->StopMovement();
		}
	}
}

void AMagicultEnemy::OnStunnedChanged(bool bNewStunned)
{
	if (MagicultAIController && MagicultAIController->GetBlackboardComponent())
	{
		MagicultAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bNewStunned);
		if (bNewStunned)
		{
			MagicultAIController->StopMovement();
		}
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
				// 풀피에서 주기 회복 GE가 적용돼도 PostGameplayEffectExecute의 Clamp로 값이
				// 그대로면(OldValue==NewValue) 굳이 UI를 다시 갱신하지 않는다.
				if (FMath::IsNearlyEqual(Data.NewValue, Data.OldValue)) return;
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
	if (!bWaitForActivationTrigger)
	{
		StartBehaviorTree();
	}
}

void AMagicultEnemy::ActivateCombat()
{
	if (!HasAuthority()) return;
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
