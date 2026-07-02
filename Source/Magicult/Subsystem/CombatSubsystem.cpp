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
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "ProfilingDebugging/CountersTrace.h"

// TriggerQueue 처리 비용 관측용 계측. Test/Shipping 빌드에서는 엔진이 자동으로 컴파일 아웃한다.
//
// 두 경로로 내보낸다:
//  1) `stat Magicult` 오버레이 + Insights Timing(stats 채널) : DECLARE_*_STAT / SCOPE_CYCLE_COUNTER
//  2) Insights Counters 트랙 (counters 채널)                : TRACE_DECLARE_INT_COUNTER / TRACE_COUNTER_SET
//     (레거시 STAT 카운터는 Counters 트랙으로 흐르지 않으므로 별도로 필요하다)
DECLARE_STATS_GROUP(TEXT("Magicult"), STATGROUP_Magicult, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("ProcessTriggerQueue (drain)"), STAT_Magicult_ProcessTriggerQueue, STATGROUP_Magicult);
DECLARE_CYCLE_STAT(TEXT("  CollectActions"), STAT_Magicult_CollectActions, STATGROUP_Magicult);
DECLARE_CYCLE_STAT(TEXT("  ExecuteRequest"), STAT_Magicult_ExecuteRequest, STATGROUP_Magicult);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Triggers processed / frame"), STAT_Magicult_TriggersProcessed, STATGROUP_Magicult);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Triggers generated / frame"), STAT_Magicult_TriggersGenerated, STATGROUP_Magicult);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Max chain depth / frame"), STAT_Magicult_MaxChainDepth, STATGROUP_Magicult);

TRACE_DECLARE_INT_COUNTER(Magicult_TriggersProcessed, TEXT("Magicult/Triggers Processed"));
TRACE_DECLARE_INT_COUNTER(Magicult_TriggersGenerated, TEXT("Magicult/Triggers Generated"));
TRACE_DECLARE_INT_COUNTER(Magicult_MaxChainDepth, TEXT("Magicult/Max Chain Depth"));

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
    // 큐 드레인 전체 = Insights CPU 타임라인의 한 이벤트 + `stat Magicult` 의 한 줄.
    SCOPE_CYCLE_COUNTER(STAT_Magicult_ProcessTriggerQueue);
    TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ProcessTriggerQueue);

    static constexpr int32 MaxChainDepth = 8;

    int32 ProcessedThisFrame = 0;
    int32 GeneratedThisFrame = 0;
    int32 MaxDepthThisFrame = 0;

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

        // 실제로 실행되는 트리거 1건 = 타임라인에 개별 이벤트로 보이도록 스코프를 건다.
        // 아래 하위 스코프들이 이 블록 내부를 빈틈없이 채운다.
        TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ProcessTrigger);
        ++ProcessedThisFrame;
        MaxDepthThisFrame = FMath::Max(MaxDepthThisFrame, Trigger.ChainDepth);

        TArray<FSpellActionSpec> Actions;
        {
            SCOPE_CYCLE_COUNTER(STAT_Magicult_CollectActions);
            TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_CollectActions);
            FSpellTriggerMatcher::CollectActions(ActionSourceInterface->GetTriggerActionBindings(), Trigger, Actions);
        }

        UActionExecutorSubsystem* Executor = GetWorld()->GetSubsystem<UActionExecutorSubsystem>();
        TArray<FSpellActionExecutionRequest> ExecutionRequests;
        {
            TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_BuildRequests);
            ExecutionRequests.Reserve(Actions.Num());
            for (const FSpellActionSpec& Action : Actions)
            {
                FSpellActionExecutionRequest ExecutionRequest;
                ExecutionRequest.Context = Trigger.SpellContext;
                ExecutionRequest.Action = Action;
                ExecutionRequests.Add(ExecutionRequest);
            }
        }

        TArray<FSpellTriggerEventData> GeneratedTriggers;
        {
            SCOPE_CYCLE_COUNTER(STAT_Magicult_ExecuteRequest);
            TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ExecuteRequest);
            GeneratedTriggers = Executor->ExecuteRequest(ExecutionRequests);
        }
        GeneratedThisFrame += GeneratedTriggers.Num();

        // 파생 트리거에 부모 깊이 +1을 실어 전파
        {
            TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_EnqueueGenerated);
            for (FSpellTriggerEventData NewTrigger : GeneratedTriggers)
            {
                NewTrigger.ChainDepth = Trigger.ChainDepth + 1;
                TriggerQueue.Enqueue(NewTrigger);
            }
        }
    }

    // 프레임당 처리량 지표.
    // stats 채널 → `stat Magicult` 오버레이 / Insights Timing 하단
    SET_DWORD_STAT(STAT_Magicult_TriggersProcessed, ProcessedThisFrame);
    SET_DWORD_STAT(STAT_Magicult_TriggersGenerated, GeneratedThisFrame);
    SET_DWORD_STAT(STAT_Magicult_MaxChainDepth, MaxDepthThisFrame);
    // counters 채널 → Insights Counters 트랙 (그래프). 매 프레임 세팅해 연속 그래프가 되게 한다.
    TRACE_COUNTER_SET(Magicult_TriggersProcessed, ProcessedThisFrame);
    TRACE_COUNTER_SET(Magicult_TriggersGenerated, GeneratedThisFrame);
    TRACE_COUNTER_SET(Magicult_MaxChainDepth, MaxDepthThisFrame);
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