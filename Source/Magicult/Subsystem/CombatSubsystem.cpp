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
    static constexpr int32 MaxChainDepth = 8;

    while (!TriggerQueue.IsEmpty())
    {
        FSpellTriggerEventData Trigger;
        TriggerQueue.Dequeue(Trigger);

        // 깊이 초과 트리거는 skip - break가 아니라 continue로 큐를 끝까지 소진한다.
        // (독립적인 원본 트리거들이 누락되지 않도록)
        if (Trigger.ChainDepth > MaxChainDepth)
        {
            UE_LOG(LogTemp, Warning, TEXT("SpellTrigger: MaxChainDepth(%d) exceeded. TriggerTag=%s"),
                MaxChainDepth, *Trigger.TriggerTag.ToString());
            continue;
        }

        if (!Trigger.SpellContext.DamageCauser.IsValid()) continue;

        IActionSourceInterface* ActionSourceInterface = Cast<IActionSourceInterface>(Trigger.SpellContext.DamageCauser);
        if (!ActionSourceInterface) continue;

        TArray<FSpellActionSpec> Actions;
        FSpellTriggerMatcher::CollectActions(ActionSourceInterface->GetTriggerActionBindings(), Trigger, Actions);

        UActionExecutorSubsystem* Executor = GetWorld()->GetSubsystem<UActionExecutorSubsystem>();
        TArray<FSpellActionExecutionRequest> ExecutionRequests;
        for (const FSpellActionSpec& Action : Actions)
        {
            FSpellActionExecutionRequest ExecutionRequest;
            ExecutionRequest.Context = Trigger.SpellContext;
            ExecutionRequest.Action = Action;
            ExecutionRequests.Add(ExecutionRequest);
        }

        TArray<FSpellTriggerEventData> GeneratedTriggers = Executor->ExecuteRequest(ExecutionRequests);

        // 파생 트리거에 부모 깊이 +1을 실어 전파
        for (FSpellTriggerEventData NewTrigger : GeneratedTriggers)
        {
            NewTrigger.ChainDepth = Trigger.ChainDepth + 1;
            TriggerQueue.Enqueue(NewTrigger);
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