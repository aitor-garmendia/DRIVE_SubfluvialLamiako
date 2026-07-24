#include "Scenario/FScenarioRuntime.h"
#include "Simulation/FSimulationClock.h"
#include "Messaging/FDriveMessageBus.h"
#include "Systems/DriveSystemBase.h"
#include "Systems/DriveSystemContext.h"
#include "ECS/DriveEcsWorld.h"
#include "Scenario/DriveScenarioStateBuilder.h"
#include "Simulation/DriveSimulationEvents.h"
#include "Logging/DriveLog.h"

FScenarioRuntime::FScenarioRuntime() = default;
FScenarioRuntime::~FScenarioRuntime() = default;

void FScenarioRuntime::Initialize(FSimulationClock* InClock, FDriveMessageBus* InBus)
{
    Clock = InClock;
    Bus = InBus;
    bInitialized = (Clock != nullptr && Bus != nullptr);

	// Stats
	Stats.bInitialized = bInitialized;
	Stats.State = EScenarioRuntimeState::Stopped;
	Stats.bScenarioLoaded = bScenarioLoaded;
	Stats.bScenarioLocked = bScenarioLocked;
	Stats.RegisteredSystems = SystemsById.Num();
	Stats.ActiveFixedTickSystems = ExecutionPlan_FixedTick.Num();

	EcsWorld = MakeUnique<FDriveEcsWorld>();

	DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] Initialized | Clock=%s Bus=%s"), Clock ? TEXT("OK") : TEXT("NULL"), Bus   ? TEXT("OK") : TEXT("NULL"));
    State = EScenarioRuntimeState::Stopped;
}

void FScenarioRuntime::SetState(EScenarioRuntimeState NewState)
{
	const EScenarioRuntimeState OldState = State;
	const bool bInvalid =
		(OldState == EScenarioRuntimeState::Stopped && NewState == EScenarioRuntimeState::Paused) ||
		(OldState == EScenarioRuntimeState::Running && NewState == EScenarioRuntimeState::Running) ||
		(OldState == EScenarioRuntimeState::Paused  && NewState == EScenarioRuntimeState::Paused);

	if (bInvalid)
	{
		Stats.InvalidStateTransitions++;
		DRIVE_LOG_RUNTIME(Warning, TEXT("[DRIVE][Runtime] Invalid state transition %d -> %d"), (int32)OldState, (int32)NewState);
	}

    State = NewState;
	Stats.State = State;
	Stats.bInitialized = bInitialized;
	Stats.bScenarioLoaded = bScenarioLoaded;
	Stats.bScenarioLocked = bScenarioLocked;
	Stats.RegisteredSystems = SystemsById.Num();
	Stats.ActiveFixedTickSystems = ExecutionPlan_FixedTick.Num();

    const TCHAR* StateStr = (State == EScenarioRuntimeState::Running) ? TEXT("Running") : (State == EScenarioRuntimeState::Paused)  ? TEXT("Paused") : TEXT("Stopped");
    DRIVE_LOG_RUNTIME(Log, TEXT("State -> %s (%d)"), StateStr, (int32)State);

	RuntimeStateChanged.Broadcast(OldState, State);
}

void FScenarioRuntime::Start()
{
	DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] Start() called | bInitialized=%d CurrentState=%d"), bInitialized ? 1 : 0, (int32)State);
    if (!bInitialized)
    {
        DRIVE_LOG_RUNTIME(Error, TEXT("Start() aborted: not initialized"));
        return;
    }

	if (State != EScenarioRuntimeState::Stopped)
	{
		Stats.InvalidStateTransitions++;
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] Start() aborted: invalid state (must be Stopped). Current=%d"), (int32)State);
		return;
	}

	FString Error;
    if (!CanStart(Error))
    {
		DRIVE_LOG_RUNTIME(Warning, TEXT("[DRIVE][Runtime] Start blocked: %s"), *Error);
        return;
    }

	bScenarioLocked = true;
	ResetDeterminismStats();
    SetState(EScenarioRuntimeState::Running);

	Stats.bScenarioLocked = bScenarioLocked;
	Stats.FixedStepCount = FixedStepCount;
	Stats.ExecutionOrderHash = ExecutionOrderHash;
}

bool FScenarioRuntime::CanStart(FString& OutError) const
{
    OutError.Reset();

    if (!bInitialized)
    {
        OutError = TEXT("Runtime not initialized (Clock/Bus null)");
        return false;
    }

    if (!Clock)
    {
        OutError = TEXT("Clock is null");
        return false;
    }

    if (!Bus)
    {
        OutError = TEXT("MessageBus is null");
        return false;
    }

    if (!bScenarioLoaded)
    {
        OutError = TEXT("No scenario loaded");
        return false;
    }

	if (!ScenarioDef.IsValidBasic())
	{
		OutError = TEXT("Scenario definition is invalid (IsValidBasic=false)");
		return false;
	}

    if (!bHasSystemContext || !SystemContext.IsValid())
    {
        OutError = TEXT("SystemContext not set/invalid (systems not initialized)");
        return false;
    }

    if (ExecutionPlan_FixedTick.Num() == 0)
    {
        OutError = TEXT("Execution plan is empty (no systems to tick)");
        return false;
    }

    for (const TSharedRef<FDriveSystemBase>& Sys : ExecutionPlan_FixedTick)
    {
        if (!Sys->IsInitialized())
        {
            OutError = FString::Printf(TEXT("System not initialized: %s"), *Sys->GetSystemId().ToString());
            return false;
        }
    }

    return true;
}

void FScenarioRuntime::Pause()
{
    if (!bInitialized)
	{
		Stats.InvalidStateTransitions++;
		return;
	}

	if (State != EScenarioRuntimeState::Running)
	{
		Stats.InvalidStateTransitions++;
		DRIVE_LOG_RUNTIME(Warning, TEXT("[DRIVE][Runtime] Pause() ignored (state != Running). Current=%d"), (int32)State);
		return;
	}

	ResetDeterminismStats();
    SetState(EScenarioRuntimeState::Paused);
}

void FScenarioRuntime::Stop()
{
    if (!bInitialized) return;

	for (TSharedRef<FDriveSystemBase>& Sys : Systems)
    {
        if (Sys->IsInitialized())
        {
			DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] System Shutdown: %s"), *Sys->GetSystemId().ToString());
            Sys->Shutdown();
        }
    }
	ResetDeterminismStats();
    SetState(EScenarioRuntimeState::Stopped);
	UnlockScenario();

	Stats.bScenarioLocked = bScenarioLocked;
	Stats.bScenarioLoaded = bScenarioLoaded;
	Stats.RegisteredSystems = SystemsById.Num();
	Stats.ActiveFixedTickSystems = ExecutionPlan_FixedTick.Num();
}

void FScenarioRuntime::Reset()
{
    if (!bInitialized) return;

    ActiveAgents.Reset();
    Systems.Reset();
    UnregisterAllSystems();
	SimState.Reset();

	bScenarioLoaded = false;
	ScenarioDef = FDriveScenarioDefinition{};
	ScenarioLoadResult.Reset();
	ResetDeterminismStats();
	SetState(EScenarioRuntimeState::Stopped);
	UnlockScenario();
	ResetRuntimeStats();
}

void FScenarioRuntime::Resume()
{
	DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] Resume() called | bInitialized=%d CurrentState=%d"),
		bInitialized ? 1 : 0, (int32)State);

	if (!bInitialized)
	{
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] Resume() aborted: not initialized"));
		return;
	}

	if (State != EScenarioRuntimeState::Paused)
	{
		DRIVE_LOG_RUNTIME(Verbose, TEXT("[DRIVE][Runtime] Resume() ignored: not paused"));
		return;
	}

	SetState(EScenarioRuntimeState::Running);
}

void FScenarioRuntime::UnlockScenario()
{
    bScenarioLocked = false;
}


void FScenarioRuntime::RegisterSystem(TSharedRef<FDriveSystemBase> System)
{
	SystemsById.Add(System->GetSystemId(), System);
	Systems.Add(System);
	System->OnRegistered();

	Systems.Sort([](const TSharedRef<FDriveSystemBase>& A, const TSharedRef<FDriveSystemBase>& B)
	{
		return A->GetFixedTickOrder() < B->GetFixedTickOrder();
	});

	DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] System registered: %s (InitOrder=%d, FixedOrder=%d). Total=%d"), 
		*System->GetSystemId().ToString(), System->GetInitOrder(), System->GetFixedTickOrder(), Systems.Num());

	if (bInitialized && bHasSystemContext && !System->IsInitialized())
	{
		if (bHasSystemContext)
		{
			DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] System Initialize (late): %s"), *System->GetSystemId().ToString());
			const bool bOk = System->Initialize(SystemContext);
			if (!bOk)
			{
				DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] Late init failed for system: %s"), *System->GetSystemId().ToString());
			}
		}
		else
		{
			DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] Late init requested but no valid SystemContext stored"));
		}
	}
}


static int32 GetInitOrderSafe(const TSharedRef<FDriveSystemBase>& Sys) { return Sys->GetInitOrder(); }
static int32 GetTickOrderSafe(const TSharedRef<FDriveSystemBase>& Sys) { return Sys->GetFixedTickOrder(); }

bool FScenarioRuntime::TopoSortSystems(TArray<TSharedRef<FDriveSystemBase>>& OutSortedInit) const
{
	OutSortedInit.Reset();
	TMap<FName, int32> InDegree;
	TMap<FName, TArray<FName>> Graph;

	for (const auto& Pair : SystemsById)
	{
		InDegree.Add(Pair.Key, 0);
		Graph.Add(Pair.Key, {});
	}

	for (const auto& Pair : SystemsById)
	{
		const FName SysId = Pair.Key;
		TArray<FName> Deps;
		Pair.Value->GetDependencies(Deps);

		for (const FName Dep : Deps)
		{
			if (!SystemsById.Contains(Dep))
			{
				DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] Missing dependency: %s depends on %s"), *SysId.ToString(), *Dep.ToString());
				return false;
			}
			Graph[Dep].Add(SysId);
			InDegree[SysId] += 1;
		}
	}

	TArray<FName> Ready;
	for (const auto& Pair : InDegree)
	{
		if (Pair.Value == 0)
		{
			Ready.Add(Pair.Key);
		}
	}

	Ready.Sort([this](const FName& A, const FName& B)
	{
		return SystemsById[A]->GetInitOrder() < SystemsById[B]->GetInitOrder();
	});

	while (Ready.Num() > 0)
	{
		const FName Current = Ready[0];
		Ready.RemoveAt(0);

		OutSortedInit.Add(SystemsById[Current]);

		for (const FName& Next : Graph[Current])
		{
			InDegree[Next] -= 1;
			if (InDegree[Next] == 0)
			{
				Ready.Add(Next);
				Ready.Sort([this](const FName& A, const FName& B)
				{
					return SystemsById[A]->GetInitOrder() < SystemsById[B]->GetInitOrder();
				});
			}
		}
	}

	if (OutSortedInit.Num() != SystemsById.Num())
	{
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] Dependency cycle detected (TopoSort failed)"));
		return false;
	}

	return true;
}

bool FScenarioRuntime::BuildExecutionPlanAndInitializeSystems(const FDriveSystemContext& Context)
{
	if (!bInitialized)
	{
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] BuildExecutionPlan failed: Runtime not initialized"));
		return false;
	}

	if (!Context.IsValid())
	{
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] BuildExecutionPlan failed: Context invalid"));
		return false;
	}

    SystemContext = Context;
    bHasSystemContext = true;

	TArray<TSharedRef<FDriveSystemBase>> SortedInit;
	if (!TopoSortSystems(SortedInit))
	{
		return false;
	}

	if (bScenarioLoaded)
	{
		TSet<FName> Enabled;
		Enabled.Reserve(ScenarioDef.Systems.EnabledSystems.Num());
		for (const FName& Id : ScenarioDef.Systems.EnabledSystems)
		{
			Enabled.Add(Id);
		}

		SortedInit.RemoveAll([&Enabled](const TSharedRef<FDriveSystemBase>& Sys)
		{
			return !Enabled.Contains(Sys->GetSystemId());
		});
	}

	for (TSharedRef<FDriveSystemBase>& Sys : SortedInit)
	{
		const bool bOk = Sys->Initialize(Context);
		DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] Init %s -> %s"), *Sys->GetSystemId().ToString(), bOk ? TEXT("OK") : TEXT("FAIL"));

		if (!bOk)
		{
			return false;
		}
	}

	ExecutionPlan_FixedTick = SortedInit;
	ExecutionPlan_FixedTick.StableSort([](const TSharedRef<FDriveSystemBase>& A, const TSharedRef<FDriveSystemBase>& B)
	{
		return A->GetFixedTickOrder() < B->GetFixedTickOrder();
	});

	DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] Systems initialized=%d | FixedTickPlan=%d"), SortedInit.Num(), ExecutionPlan_FixedTick.Num());

	// Stats sync
	Stats.RegisteredSystems = SystemsById.Num();
	Stats.ActiveFixedTickSystems = ExecutionPlan_FixedTick.Num();
	Stats.LastSystemTickWallSeconds.SetNumZeroed(ExecutionPlan_FixedTick.Num());

	if (!bScenarioLoaded)
	{
		if (SystemsById.Num() == 0)
		{
			DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] No Scenario loaded, but no systems registered yet -> implicit scenario deferred"));
		}
		else
		{
			ScenarioDef = FDriveScenarioDefinition{};

			ScenarioDef.Header.ScenarioId = TEXT("Implicit");
			ScenarioDef.Header.ScenarioGuid = FGuid::NewGuid();
			ScenarioDef.Header.DefinitionVersion = 1;
			ScenarioDef.Header.DisplayName = TEXT("Implicit Scenario");
			ScenarioDef.Header.Author = TEXT("Runtime");
			ScenarioDef.Header.Description = TEXT("Auto-generated from registered systems (compat it1-it3).");

			ScenarioDef.Systems.EnabledSystems.Reset();
			ScenarioDef.Systems.EnabledSystems.Reserve(SystemsById.Num());

			for (const auto& Pair : SystemsById)
			{
				ScenarioDef.Systems.EnabledSystems.Add(Pair.Key);
			}

			bScenarioLoaded = true;

			DRIVE_LOG_RUNTIME(Warning, TEXT("[DRIVE][Runtime] No Scenario loaded -> using implicit scenario (ScenarioId=%s, EnabledSystems=%d)"),
				*ScenarioDef.Header.ScenarioId.ToString(), ScenarioDef.Systems.EnabledSystems.Num());
		}
	}

	if (bScenarioLoaded && ScenarioDef.Header.ScenarioId == TEXT("Implicit") && ScenarioDef.Systems.EnabledSystems.Num() == 0 && SystemsById.Num() > 0)
	{
		ScenarioDef.Systems.EnabledSystems.Reserve(SystemsById.Num());
		for (const auto& Pair : SystemsById)
		{
			ScenarioDef.Systems.EnabledSystems.Add(Pair.Key);
		}

		DRIVE_LOG_RUNTIME(Warning, TEXT("[DRIVE][Runtime] Implicit scenario was empty -> regenerated (EnabledSystems=%d)"), ScenarioDef.Systems.EnabledSystems.Num());
	}

	return true;
}

int32 FScenarioRuntime::Advance(float RealDeltaSeconds)
{
	const double AdvanceStart = FPlatformTime::Seconds();
	Stats.LastAdvanceRealDeltaSeconds = RealDeltaSeconds;
	Stats.LastAdvanceFixedStepsExecuted = 0;
	Stats.LastAdvanceWallSeconds = 0.0;

    if (!bInitialized)
    {
		DRIVE_LOG_RUNTIME(VeryVerbose, TEXT("[DRIVE][Runtime] Advance skipped (not initialized)"));
        return 0;
    }

    if (State != EScenarioRuntimeState::Running)
    {
		DRIVE_LOG_RUNTIME(VeryVerbose, TEXT("[DRIVE][Runtime] Advance skipped (state != Running)"));
        return 0;
    }

    if (!Clock)
    {
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] Advance failed: Clock is null"));
        return 0;
    }

	if (!bHasSystemContext || !SystemContext.IsValid() || ExecutionPlan_FixedTick.Num() == 0)
	{
		Stats.InvalidAdvanceCalls++;
		DRIVE_LOG_RUNTIME(Warning, TEXT("Advance blocked: Running but context/plan invalid (HasCtx=%d CtxValid=%d Plan=%d)"),
			bHasSystemContext ? 1 : 0, SystemContext.IsValid() ? 1 : 0, ExecutionPlan_FixedTick.Num());
		return 0;
	}


	Clock->TickRealTime(RealDeltaSeconds);
	const int32 Steps = Clock->GetPendingFixedSteps();
    if (Steps <= 0)
    {
        return 0;
    }

    const float FixedDt = Clock->GetFixedDeltaTimeSeconds();
    for (int32 i = 0; i < Steps; ++i)
    {
        if (Bus)
        {
            FDriveEvent_FixedStep Ev;
            Ev.StepIndexThisFrame = i;
            Ev.FixedDt = FixedDt;
            Ev.SimTimeSeconds = Clock->GetSimTimeSeconds();
            Bus->Enqueue(Ev);
        }

        AdvanceFixedStep_Internal(FixedDt);
        const bool bConsumed = Clock->ConsumeFixedStep();
        ensureMsgf(bConsumed, TEXT("[DRIVE][Runtime] ConsumeFixedStep failed unexpectedly (accumulator underflow)"));
    }
	
	Stats.LastAdvanceFixedDt = FixedDt;

	Stats.LastAdvanceFixedStepsExecuted = Steps;
	Stats.LastAdvanceWallSeconds = FPlatformTime::Seconds() - AdvanceStart;
	Stats.FixedStepCount = FixedStepCount;
	Stats.ExecutionOrderHash = ExecutionOrderHash;
	Stats.RegisteredSystems = SystemsById.Num();
	Stats.ActiveFixedTickSystems = ExecutionPlan_FixedTick.Num();


    return Steps;
}

void FScenarioRuntime::AddAgent(TSharedPtr<FAgentBase> Agent)
{
    if (!Agent) return;
    ActiveAgents.Add(Agent);
}

void FScenarioRuntime::RemoveAgent(const FGuid& AgentId)
{
    ActiveAgents.RemoveAll([&](const TSharedPtr<FAgentBase>& A)
    {
        return false;
    });
}

void FScenarioRuntime::AdvanceFixedStep_Internal(float FixedDt)
{
	const double FixedStepStart = FPlatformTime::Seconds();
	if (Stats.LastSystemTickWallSeconds.Num() != ExecutionPlan_FixedTick.Num())
	{
		Stats.LastSystemTickWallSeconds.SetNumZeroed(ExecutionPlan_FixedTick.Num());
	}
	else
	{
		for (double& V : Stats.LastSystemTickWallSeconds) { V = 0.0; }
	}

    if (!bInitialized)
    {
		DRIVE_LOG_RUNTIME(VeryVerbose, TEXT("[DRIVE][Runtime] Advance skipped (not initialized)"));
        return;
    }

    if (State != EScenarioRuntimeState::Running)
    {
		DRIVE_LOG_RUNTIME(VeryVerbose, TEXT("[DRIVE][Runtime] Advance skipped (state != Running)"));
        return;
    }
	++FixedStepCount;

	// ---- DETERMINISM SUMMARY ----
    constexpr uint64 LogEveryN = 60; 
    if ((FixedStepCount % LogEveryN) == 0)
    {
        const double SimT  = Clock ? Clock->GetSimTimeSeconds() : 0.0;
        const double RealT = Clock ? Clock->GetRealTimeSeconds() : 0.0;
        const double AccT  = Clock ? Clock->GetAccumulatedRealSeconds() : 0.0;

		DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Determinism] FixedSteps=%llu SimTime=%.6f RealTime=%.6f Acc=%.6f ExecHash=0x%08x"),
            (unsigned long long)FixedStepCount, SimT, RealT, AccT, ExecutionOrderHash);
    }

	for (const TSharedRef<FDriveSystemBase>& Sys : ExecutionPlan_FixedTick)
	{
		ExecutionOrderHash = HashCombineFast(ExecutionOrderHash, GetTypeHash(Sys->GetSystemId()));
	}
	ExecutionOrderHash = HashCombineFast(ExecutionOrderHash, ::GetTypeHash(FixedStepCount));

	if (EcsWorld)
    {
        EcsWorld->BeginFixedTick(); 
    }

	for (int32 Idx = 0; Idx < ExecutionPlan_FixedTick.Num(); ++Idx)
	{
		TSharedRef<FDriveSystemBase>& Sys = ExecutionPlan_FixedTick[Idx];

		if (!Sys->IsInitialized())
		{
			Stats.SkippedSystemTicks++;
			DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] Skipping TickFixed: system not initialized: %s"), *Sys->GetDebugName());
			continue;
		}

		const double SysStart = FPlatformTime::Seconds();
		Sys->TickFixed(FixedDt);
		const double SysEnd = FPlatformTime::Seconds();
		Stats.LastSystemTickWallSeconds[Idx] = (SysEnd - SysStart);
	}

	if (Bus) { Bus->Flush(); }

	for (TSharedRef<FDriveSystemBase>& Sys : ExecutionPlan_FixedTick)
	{
		if (!Sys->IsInitialized()) { continue; }
		Sys->PostTickFixed(FixedDt);
	}

	if (EcsWorld)
    {
        EcsWorld->EndFixedTick();
    }

	const double FixedStepEnd = FPlatformTime::Seconds();
	Stats.LastFixedStepWallSeconds = (FixedStepEnd - FixedStepStart);
	Stats.TotalFixedStepWallSeconds += Stats.LastFixedStepWallSeconds;

	Stats.FixedStepCount = FixedStepCount;
	Stats.ExecutionOrderHash = ExecutionOrderHash;
	Stats.RegisteredSystems = SystemsById.Num();
	Stats.ActiveFixedTickSystems = ExecutionPlan_FixedTick.Num();
}

bool FScenarioRuntime::LoadScenarioDefinition(const FDriveScenarioDefinition& InDef)
{
	if (!bInitialized)
	{
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] LoadScenarioDefinition failed: runtime not initialized"));
		return false;
	}

	if (State != EScenarioRuntimeState::Stopped || bScenarioLocked)
	{
		DRIVE_LOG_RUNTIME(Warning, TEXT("[DRIVE][Runtime] LoadScenarioDefinition rejected: scenario is locked (state=%d)"), (int32)State);
		return false;
	}

	ScenarioLoadResult.Reset();

	if (!InDef.IsValidBasic())
	{
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] LoadScenarioDefinition failed: invalid definition"));
		return false;
	}

	ScenarioLoadResult.bDefinitionValid = true;
	ScenarioLoadResult.ScenarioId = InDef.Header.ScenarioId;

	ScenarioDef = InDef;
	bScenarioLoaded = true;

	TSet<FName> Seen;
	for (const FName& SysId : InDef.Systems.EnabledSystems)
	{
		if (SysId.IsNone())
		{
			continue;
		}

		if (!Seen.Contains(SysId))
		{
			Seen.Add(SysId);
			ScenarioLoadResult.RequestedSystems.Add(SysId);
		}
	}

	for (const FName SysId : ScenarioLoadResult.RequestedSystems)
	{
		if (!SystemsById.Contains(SysId))
		{
			ScenarioLoadResult.MissingSystems.Add(SysId);
		}
	}

	DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] Scenario loaded: %s | RequestedSystems=%d | Missing=%d"),
	*ScenarioLoadResult.ScenarioId.ToString(),
	ScenarioLoadResult.RequestedSystems.Num(),
	ScenarioLoadResult.MissingSystems.Num());

	ScenarioLoaded.Broadcast(ScenarioLoadResult.ScenarioId);
	return true;
}

void FScenarioRuntime::UnregisterAllSystems()
{
    for (TSharedRef<FDriveSystemBase>& Sys : Systems)
    {
        if (Sys->IsInitialized())
        {
			DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] System Shutdown (unregister): %s"), *Sys->GetSystemId().ToString());
            Sys->Shutdown();
        }
    }

    Systems.Reset();               
    SystemsById.Reset();
    ExecutionPlan_FixedTick.Reset();
    bHasSystemContext = false;       
}

bool FScenarioRuntime::BuildInitialStateFromScenario()
{
	if (!bInitialized)
	{
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] BuildInitialStateFromScenario failed: runtime not initialized"));
		return false;
	}

	if (!bScenarioLoaded)
	{
		DRIVE_LOG_RUNTIME(Error, TEXT("[DRIVE][Runtime] BuildInitialStateFromScenario failed: no scenario loaded"));
		return false;
	}

	bInitialStateBuilt = false;
	InitialState = FDriveScenarioInitState{};

	const bool bOk = FDriveScenarioStateBuilder::BuildInitialState(ScenarioDef, InitialState);
	bInitialStateBuilt = bOk;

	if (bOk)
	{
		DRIVE_LOG_RUNTIME(Log, TEXT("[DRIVE][Runtime] InitialState built | Scenario=%s | Seed=%u | Templates=%d | Spawns=%d | Globals=%d"),
			*InitialState.ScenarioId.ToString(), InitialState.Seed, InitialState.EntityTemplates.Num(), InitialState.Spawns.Num(), InitialState.GlobalParamsSorted.Num());
	}

	return bOk;
}

void FScenarioRuntime::ResetDeterminismStats()
{
    FixedStepCount = 0;
    ExecutionOrderHash = 0;
	Stats.FixedStepCount = FixedStepCount;
	Stats.ExecutionOrderHash = ExecutionOrderHash;
}

void FScenarioRuntime::ResetRuntimeStats()
{
	Stats = FScenarioRuntimeStats{};
	Stats.bInitialized = bInitialized;
	Stats.bScenarioLoaded = bScenarioLoaded;
	Stats.bScenarioLocked = bScenarioLocked;
	Stats.State = State;

	Stats.RegisteredSystems = SystemsById.Num();
	Stats.ActiveFixedTickSystems = ExecutionPlan_FixedTick.Num();
	Stats.FixedStepCount = FixedStepCount;
	Stats.ExecutionOrderHash = ExecutionOrderHash;
}

void FScenarioRuntime::GetLastFixedTickSystemIds(TArray<FName>& OutIds) const
{
	OutIds.Reset();
	OutIds.Reserve(ExecutionPlan_FixedTick.Num());
	for (const TSharedRef<FDriveSystemBase>& Sys : ExecutionPlan_FixedTick)
	{
		OutIds.Add(Sys->GetSystemId());
	}
}

void FScenarioRuntime::GetLastFixedTickSystemTimes(TArray<double>& OutSeconds) const
{
	OutSeconds = Stats.LastSystemTickWallSeconds;
}

void FScenarioRuntime::GetRegisteredSystemIds(TArray<FName>& OutIds) const
{
	OutIds.Reset(Systems.Num());
	for (const TSharedRef<FDriveSystemBase>& Sys : Systems)
	{
		OutIds.Add(Sys->GetSystemId());
	}
}

void FScenarioRuntime::GetExecutionPlanSystemIds(TArray<FName>& OutIds) const
{
	OutIds.Reset(ExecutionPlan_FixedTick.Num());
	for (const TSharedRef<FDriveSystemBase>& Sys : ExecutionPlan_FixedTick)
	{
		OutIds.Add(Sys->GetSystemId());
	}
}

int32 FScenarioRuntime::AdvanceFixedSteps_Manual(int32 NumSteps)
{
    if (NumSteps <= 0)
    {
        return 0;
    }

    if (!bInitialized || !Clock)
    {
        return 0;
    }

    // No permitimos step manual si está Stopped
    if (State == EScenarioRuntimeState::Stopped)
    {
        return 0;
    }

    // Guardamos estado actual (normalmente Paused) y forzamos Running solo para ejecutar steps
    const EScenarioRuntimeState SavedState = State;
    State = EScenarioRuntimeState::Running;

    int32 Total = 0;
    const float FixedDt = Clock->GetFixedDeltaTimeSeconds();

    for (int32 i = 0; i < NumSteps; ++i)
    {
        const int32 Steps = Advance(FixedDt);
        if (Steps <= 0)
        {
            break;
        }
        Total += Steps;
    }

    // Restauramos estado previo (normalmente Paused)
    State = SavedState;
    return Total;
}
