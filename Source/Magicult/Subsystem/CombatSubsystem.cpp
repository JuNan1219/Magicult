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
#include "HAL/IConsoleManager.h"

// 프레임당 처리할 Action 개수 상한(트리거 단위로 체크 - ProcessTriggerQueue 참조).
// 다발 폭발/원소 연쇄로 인한 500+ Action/frame 스파이크를 여러 프레임에 걸쳐 분산시키기 위한 값.
// 하드 데드라인이 아니라 스파이크 완화용 휴리스틱 — 트리거 하나의 액션 배치는 항상 원자적으로
// 같은 프레임에서 끝까지 처리되므로(데미지/이펙트 불일치 방지) 예산을 다소 오버슈트할 수 있다.
static TAutoConsoleVariable<int32> CVarMaxActionsPerFrame(
    TEXT("Magicult.Combat.MaxActionsPerFrame"),
    128,
    TEXT("한 프레임에 처리할 최대 Action 수(트리거 단위 휴리스틱, 오버슈트 가능)"),
    ECVF_Default);

// TriggerQueue 처리 비용 관측용 계측(Test/Shipping은 자동 컴파일 아웃). 두 경로로 내보낸다:
// 1) `stat Magicult`/Insights Timing(stats 채널) - DECLARE_*_STAT/SCOPE_CYCLE_COUNTER
// 2) Insights Counters 트랙(counters 채널) - TRACE_DECLARE_INT_COUNTER/TRACE_COUNTER_SET
//    (레거시 STAT 카운터는 Counters 트랙으로 안 흐르므로 별도 필요)
DECLARE_STATS_GROUP(TEXT("Magicult"), STATGROUP_Magicult, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("ProcessTriggerQueue (drain)"), STAT_Magicult_ProcessTriggerQueue, STATGROUP_Magicult);
DECLARE_CYCLE_STAT(TEXT("  CollectActions"), STAT_Magicult_CollectActions, STATGROUP_Magicult);
DECLARE_CYCLE_STAT(TEXT("  ExecuteRequest"), STAT_Magicult_ExecuteRequest, STATGROUP_Magicult);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Triggers processed / frame"), STAT_Magicult_TriggersProcessed, STATGROUP_Magicult);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Triggers generated / frame"), STAT_Magicult_TriggersGenerated, STATGROUP_Magicult);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Max chain depth / frame"), STAT_Magicult_MaxChainDepth, STATGROUP_Magicult);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Actions processed / frame"), STAT_Magicult_ActionsProcessed, STATGROUP_Magicult);

TRACE_DECLARE_INT_COUNTER(Magicult_TriggersProcessed, TEXT("Magicult/Triggers Processed"));
TRACE_DECLARE_INT_COUNTER(Magicult_TriggersGenerated, TEXT("Magicult/Triggers Generated"));
TRACE_DECLARE_INT_COUNTER(Magicult_MaxChainDepth, TEXT("Magicult/Max Chain Depth"));
TRACE_DECLARE_INT_COUNTER(Magicult_ActionsProcessed, TEXT("Magicult/Actions Processed"));

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
    EnqueueInternal(NewTriggerEventData);
}

void UCombatSubsystem::EnqueueInternal(FSpellTriggerEventData& TriggerEventData)
{
    // causer 수명 pin: 트리거가 큐에 있는 동안 DamageCauser를 GC로부터 붙잡는다.
    // action-spawned AttackInstance처럼 아무도 강참조하지 않는 causer는 이 pin이 유일한 수명줄.
    if (!TriggerEventData.DamageCauserPin.IsValid())
    {
        TriggerEventData.DamageCauserPin = TStrongObjectPtr<UObject>(TriggerEventData.SpellContext.DamageCauser.Get());
    }

    TriggerQueue.Enqueue(TriggerEventData);
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

    // 프레임 예산: 트리거 단위로 체크한다(CVarMaxActionsPerFrame 참고). 예산 초과로 못 처리한
    // 트리거는 큐에 남아 다음 Tick에 이어서 처리되며, causer는 DamageCauserPin이 프레임을
    // 넘겨서도 살려준다.
    const int32 ActionBudget = CVarMaxActionsPerFrame.GetValueOnGameThread();
    int32 ActionsProcessedThisFrame = 0;

    while (ActionsProcessedThisFrame < ActionBudget && !TriggerQueue.IsEmpty())
    {
        FSpellTriggerEventData Trigger;
        TriggerQueue.Dequeue(Trigger);

        // 깊이 초과 트리거는 skip - break가 아니라 continue로 큐를 끝까지 소진한다.
        // (독립적인 원본 트리거들이 누락되지 않도록)
        if (Trigger.SpellContext.TriggerChainDepth > MaxChainDepth)
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
        MaxDepthThisFrame = FMath::Max(MaxDepthThisFrame, Trigger.SpellContext.TriggerChainDepth);

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
        int32 ActionsExecuted = 0;
        {
            SCOPE_CYCLE_COUNTER(STAT_Magicult_ExecuteRequest);
            TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_ExecuteRequest);
            GeneratedTriggers = Executor->ExecuteRequest(ExecutionRequests, ActionsExecuted);
        }
        GeneratedThisFrame += GeneratedTriggers.Num();
        ActionsProcessedThisFrame += ActionsExecuted;

        // 파생 트리거에 부모 깊이 +1을 실어 전파
        {
            TRACE_CPUPROFILER_EVENT_SCOPE(Magicult_EnqueueGenerated);
            for (FSpellTriggerEventData NewTrigger : GeneratedTriggers)
            {
                NewTrigger.SpellContext.TriggerChainDepth = Trigger.SpellContext.TriggerChainDepth + 1;
                EnqueueInternal(NewTrigger);
            }
        }
    }

    // 프레임당 처리량 지표.
    // stats 채널 → `stat Magicult` 오버레이 / Insights Timing 하단
    SET_DWORD_STAT(STAT_Magicult_TriggersProcessed, ProcessedThisFrame);
    SET_DWORD_STAT(STAT_Magicult_TriggersGenerated, GeneratedThisFrame);
    SET_DWORD_STAT(STAT_Magicult_MaxChainDepth, MaxDepthThisFrame);
    SET_DWORD_STAT(STAT_Magicult_ActionsProcessed, ActionsProcessedThisFrame);
    // counters 채널 → Insights Counters 트랙 (그래프). 매 프레임 세팅해 연속 그래프가 되게 한다.
    TRACE_COUNTER_SET(Magicult_TriggersProcessed, ProcessedThisFrame);
    TRACE_COUNTER_SET(Magicult_TriggersGenerated, GeneratedThisFrame);
    TRACE_COUNTER_SET(Magicult_MaxChainDepth, MaxDepthThisFrame);
    TRACE_COUNTER_SET(Magicult_ActionsProcessed, ActionsProcessedThisFrame);
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