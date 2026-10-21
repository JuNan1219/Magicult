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

// 카운트 누수가 액터 영구 잔존으로 이어지지 않게 하는 안전망. 기본값은 MaxChainDepth에 맞췄다.
static TAutoConsoleVariable<int32> CVarMaxDestroyDeferFrames(
    TEXT("Magicult.Combat.MaxDestroyDeferFrames"),
    8,
    TEXT("causer 파괴를 미룰 수 있는 최대 프레임 수(초과 시 강제 파괴)"),
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
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Triggers dropped / frame"), STAT_Magicult_TriggersDropped, STATGROUP_Magicult);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Destroy deferred / frame"), STAT_Magicult_DestroyDeferred, STATGROUP_Magicult);

TRACE_DECLARE_INT_COUNTER(Magicult_TriggersProcessed, TEXT("Magicult/Triggers Processed"));
TRACE_DECLARE_INT_COUNTER(Magicult_TriggersGenerated, TEXT("Magicult/Triggers Generated"));
TRACE_DECLARE_INT_COUNTER(Magicult_MaxChainDepth, TEXT("Magicult/Max Chain Depth"));
TRACE_DECLARE_INT_COUNTER(Magicult_ActionsProcessed, TEXT("Magicult/Actions Processed"));
TRACE_DECLARE_INT_COUNTER(Magicult_TriggersDropped, TEXT("Magicult/Triggers Dropped"));
TRACE_DECLARE_INT_COUNTER(Magicult_DestroyDeferred, TEXT("Magicult/Destroy Deferred"));

void UCombatSubsystem::Deinitialize()
{
    // 큐에 남은 트리거는 감소 경로를 타지 못하므로 카운트를 통째로 비운다.
    // 액터 파괴는 월드 정리가 맡으므로 여기서 Destroy를 부르지 않는다.
    TriggerQueue.Empty();
    PendingTriggerRefCount.Empty();
    PendingDestroyActors.Reset();
    PendingReleaseObjects.Reset();

    Super::Deinitialize();
}

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

    // 전투 파이프라인은 서버 권위다. NetDriver가 아직 없는 InitWorld 시점이지만
    // UWorld가 PendingNetGame URL로 NM_Client를 되돌려주므로 판정이 성립한다.
    return !World->IsNetMode(NM_Client);
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

    AddTriggerRef(TriggerEventData.DamageCauserPin.Get());
    TriggerQueue.Enqueue(TriggerEventData);
}

void UCombatSubsystem::AddTriggerRef(UObject* Causer)
{
    // pin이 비었다는 건 enqueue 시점에 이미 causer가 무효라는 뜻 - 보호할 대상이 없다.
    if (!Causer)
    {
        return;
    }

    ++PendingTriggerRefCount.FindOrAdd(TObjectKey<UObject>(Causer));
}

void UCombatSubsystem::ReleaseTriggerRef(UObject* Causer)
{
    if (!Causer)
    {
        return;
    }

    const TObjectKey<UObject> Key(Causer);
    if (int32* RefCount = PendingTriggerRefCount.Find(Key))
    {
        if (--(*RefCount) <= 0)
        {
            PendingTriggerRefCount.Remove(Key);
        }
    }
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
        // 재등록이 DeferredFrames를 리셋하면 유예 상한이 무한정 밀린다.
        if (!PendingDestroyActors.FindByPredicate([Actor](const FPendingDestroyEntry& Entry) { return Entry.Actor == Actor; }))
        {
            PendingDestroyActors.AddDefaulted_GetRef().Actor = Actor;
        }
    }
    else
    {
        PendingReleaseObjects.AddUnique(RuntimeObject);
    }
}

void UCombatSubsystem::FlushPendingCleanup()
{
    // Actor는 큐에 남은 트리거가 참조하지 않을 때만 실제 Destroy. 참조 중이면 상한까지 미룬다
    // (PrepareCleanup은 등록 시점에 끝나 있으므로 유예해도 게임플레이/연출에는 영향이 없다).
    const int32 MaxDeferFrames = CVarMaxDestroyDeferFrames.GetValueOnGameThread();
    int32 DeferredThisFrame = 0;

    for (int32 Index = PendingDestroyActors.Num() - 1; Index >= 0; --Index)
    {
        FPendingDestroyEntry& Entry = PendingDestroyActors[Index];
        if (!IsValid(Entry.Actor))
        {
            PendingDestroyActors.RemoveAtSwap(Index);
            continue;
        }

        const int32* RefCount = PendingTriggerRefCount.Find(TObjectKey<UObject>(Entry.Actor.Get()));
        const bool bHasPendingTrigger = RefCount && *RefCount > 0;

        if (bHasPendingTrigger && Entry.DeferredFrames < MaxDeferFrames)
        {
            ++Entry.DeferredFrames;
            ++DeferredThisFrame;
            continue;
        }

        if (bHasPendingTrigger)
        {
            UE_LOG(LogTemp, Warning, TEXT("CombatSubsystem: 파괴 유예 상한(%d) 초과. 강제 파괴. Actor=%s"),
                MaxDeferFrames, *Entry.Actor->GetName());
        }

        Entry.Actor->Destroy();
        PendingDestroyActors.RemoveAtSwap(Index);
    }

    SET_DWORD_STAT(STAT_Magicult_DestroyDeferred, DeferredThisFrame);
    TRACE_COUNTER_SET(Magicult_DestroyDeferred, DeferredThisFrame);

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
    int32 DroppedThisFrame = 0;

    // 프레임 예산: 트리거 단위로 체크한다(CVarMaxActionsPerFrame 참고). 예산 초과로 못 처리한
    // 트리거는 큐에 남아 다음 Tick에 이어서 처리되며, causer는 DamageCauserPin이 프레임을
    // 넘겨서도 살려준다.
    const int32 ActionBudget = CVarMaxActionsPerFrame.GetValueOnGameThread();
    int32 ActionsProcessedThisFrame = 0;

    while (ActionsProcessedThisFrame < ActionBudget && !TriggerQueue.IsEmpty())
    {
        FSpellTriggerEventData Trigger;
        TriggerQueue.Dequeue(Trigger);

        // 아래 continue 경로들보다 먼저 - 감소가 한 곳이어야 카운트 누수(= 액터 영구 잔존)가 없다.
        ReleaseTriggerRef(Trigger.DamageCauserPin.Get());

        // 깊이 초과 트리거는 skip - break가 아니라 continue로 큐를 끝까지 소진한다.
        // (독립적인 원본 트리거들이 누락되지 않도록)
        if (Trigger.SpellContext.TriggerChainDepth > MaxChainDepth)
        {
            UE_LOG(LogTemp, Warning, TEXT("SpellTrigger: MaxChainDepth(%d) exceeded. TriggerTag=%s"),
                MaxChainDepth, *Trigger.TriggerTag.ToString());
            ++DroppedThisFrame;
            continue;
        }

        if (!Trigger.SpellContext.DamageCauser.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("SpellTrigger: DamageCauser invalid. TriggerTag=%s"),
                *Trigger.TriggerTag.ToString());
            ++DroppedThisFrame;
            continue;
        }

        IActionSourceInterface* ActionSourceInterface = Cast<IActionSourceInterface>(Trigger.SpellContext.DamageCauser);
        if (!ActionSourceInterface)
        {
            ++DroppedThisFrame;
            continue;
        }

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
    SET_DWORD_STAT(STAT_Magicult_TriggersDropped, DroppedThisFrame);
    // counters 채널 → Insights Counters 트랙 (그래프). 매 프레임 세팅해 연속 그래프가 되게 한다.
    TRACE_COUNTER_SET(Magicult_TriggersProcessed, ProcessedThisFrame);
    TRACE_COUNTER_SET(Magicult_TriggersGenerated, GeneratedThisFrame);
    TRACE_COUNTER_SET(Magicult_MaxChainDepth, MaxDepthThisFrame);
    TRACE_COUNTER_SET(Magicult_ActionsProcessed, ActionsProcessedThisFrame);
    TRACE_COUNTER_SET(Magicult_TriggersDropped, DroppedThisFrame);
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