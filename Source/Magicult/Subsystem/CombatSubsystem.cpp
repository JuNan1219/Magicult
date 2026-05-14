// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Magicult/Component/MagicComponent.h"
#include "Magicult/Subsystem/CombatResolveSubsystem.h"
#include "Magicult/Actor/MagicultProjectile.h"
#include "Magicult/Interface/CombatInterface.h"
#include "Magicult/AbilitySystem/MagicultAbilitySystemLibrary.h"
#include "Magicult/Helper/SpellTriggerMatcher.h"
#include "Magicult/Subsystem/ActionExecutorSubsystem.h"
#include "Magicult/MagicultGameplayTags.h"
#include "Magicult/Interface/ActionSourceInterface.h"

void UCombatSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 트리거 이벤트 큐잉
	ProcessTriggerQueue();

    // 미룬 삭제 실행
    FlushPendingCleanup();
}

TStatId UCombatSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UCombatSubsystem, STATGROUP_Tickables);
}

bool UCombatSubsystem::IsTickable() const
{
    return !IsTemplate();
}

bool UCombatSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer))
    {
        return false;
    }

    const UWorld* World = Cast<UWorld>(Outer);
    if (!World)
    {
        return false;
    }

    return true;
}


void UCombatSubsystem::Enqueue(FSpellTriggerEventData NewTriggerEventData)
{
    TriggerQueue.Enqueue(NewTriggerEventData);
}

void UCombatSubsystem::RegisterPendingCleanupObject(UObject* RuntimeObject)
{
    if (!RuntimeObject)
    {
        return;
    }

    if (RuntimeObject->GetClass()->ImplementsInterface(UCombatRuntimeObject::StaticClass()))
    {
        ICombatRuntimeObject::Execute_PrepareCleanup(RuntimeObject);
    }

    if (AActor* Actor = Cast<AActor>(RuntimeObject))
    {
        PendingDestroyActors.AddUnique(Actor);
    }
    else
    {
        PendingReleaseObjects.AddUnique(RuntimeObject);
    }
}

void UCombatSubsystem::FlushPendingCleanup()
{
    // Actor는 실제 Destroy
    for (AActor* Actor : PendingDestroyActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    PendingDestroyActors.Reset();

    // UObject는 삭제하지 않음.
    // 강한 참조만 해제해서 GC 가능 상태로 둠.
    PendingReleaseObjects.Reset();
}

void UCombatSubsystem::ProcessTriggerQueue()
{
    int32 ChainDepth = 0;
    const int32 MaxChainDepth = 8;

    while (!TriggerQueue.IsEmpty())
    {
        // 무한루프 방지
        if (ChainDepth > MaxChainDepth)
        {
            UE_LOG(LogTemp, Warning, TEXT("SpellTrigger: MaxChainDepth exceeded. TriggerTag=%s"), *TriggerQueue.Peek()->TriggerTag.ToString());
            break;
        }

        FSpellTriggerEventData Trigger;
        TriggerQueue.Dequeue(Trigger);

        FSpellContext ExecutionContext = Trigger.SpellContext;
        
        //ExecutionContext.TriggerEventTag = Trigger.EventTags;

        if (!Trigger.SpellContext.DamageCauser.IsValid()) continue;

        TArray<FSpellActionSpec> Actions;
        IActionSourceInterface* ActionSourceInterface = Cast<IActionSourceInterface>(Trigger.SpellContext.DamageCauser);

        if (!ActionSourceInterface) continue;

        FSpellTriggerMatcher::CollectActions(ActionSourceInterface->GetTriggerActionBindings(), Trigger, Actions);

        UActionExecutorSubsystem* Executor = GetWorld()->GetSubsystem<UActionExecutorSubsystem>();
        TArray<FSpellActionExecutionRequest> ExecutionRequests;
        for (const auto& Action : Actions)
        {
            FSpellActionExecutionRequest ExecutionRequest;
            ExecutionRequest.Context = Trigger.SpellContext;
            ExecutionRequest.Action = Action;

            ExecutionRequests.Add(ExecutionRequest);
        }
        
        TArray<FSpellTriggerEventData> GeneratedTrigger = Executor->ExecuteRequest(ExecutionRequests);

        for (const FSpellTriggerEventData& NewTrigger : GeneratedTrigger)
        {
            TriggerQueue.Enqueue(NewTrigger);
            ChainDepth++;
        }
    }
}

bool UCombatSubsystem::IsServer()
{
    bool bIsServer = !(!GetWorld() || GetWorld()->IsNetMode(NM_Client));
    return bIsServer;
}

AMagicultProjectile* UCombatSubsystem::SpawnProjectile(FSpellContext& Context, const FSpellState& SpellState, const FTransform& SpawnTransform, float BaseDamage)
{
    if (!IsServer() || !Context.SourceAbilitySystemComponent.Get()) return nullptr;

    AActor* SourceActor = Context.SourceAbilitySystemComponent->GetAvatarActor();

    // PreSpawn
    FResolvedSpellProjectileResult ResolvedSpellProjectileResult =
        GetWorld()->GetSubsystem<UCombatResolveSubsystem>()->ResolveProjectile(Context, SpellState);
    ResolvedSpellProjectileResult.Spec.BaseDamage = BaseDamage;

    // Spawn
    TSubclassOf<AMagicultProjectile> SpawnClass = ResolvedSpellProjectileResult.Spec.ProjectileBaseClass;
    if (!SpawnClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("No data found"));
        return nullptr;
    }

    AMagicultProjectile* Projectile = GetWorld()->SpawnActorDeferred<AMagicultProjectile>(
        SpawnClass,
        SpawnTransform,
        SourceActor,
        Cast<APawn>(SourceActor),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (!Projectile) return nullptr;
    Context.DamageCauser = Projectile;
    // 탄환에게 마법 정보 전달
    Projectile->FinishSpawning(SpawnTransform);
    Projectile->SetProjectileSpawnData(Context, ResolvedSpellProjectileResult);
    return Projectile;
}