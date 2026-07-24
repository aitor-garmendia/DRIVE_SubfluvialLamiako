#include "UWorldSimulationSubsystem.h"
#include "DriveGlobalConfigAsset.h"
#include "Messaging/FDriveMessageBus.h"
#include "Simulation/DriveTestEvents.h"
#include "Systems/DriveSystem_TestLogger.h"
#include "Simulation/DriveSimulationEvents.h"
#include "Systems/FDriveDummySystem.h"
#include "Systems/DriveSystemBase.h"
#include "Systems/Tests/FDriveTestSystems.h"
#include "Logging/DriveLog.h"
#include "Debug/FDriveDebugSnapshot.h"
#include "Messaging/DriveMessageBusBridge.h"
#include "Messaging/DriveBusTestEvents.h"
#include "Systems/FDriveMovementSystem.h"
#include "Managers/DriveAgentManager.h"
#include "Managers/DrivePossessionManager.h"

#include "HAL/PlatformProcess.h"
#include "Debug/FDriveDebugCommands.h"

static double GDriveNextSnapshotLogTime = 0.0;

void UWorldSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    FDriveDebugCommands::Register();
    DRIVE_LOG_UE(Log, TEXT("[DRIVE] UWorldSimulationSubsystem Initialize"));
    
    MessageBus = MakeUnique<FDriveMessageBus>();
    this->MessageBusBridge = NewObject<UDriveMessageBusBridge>(this);
    this->MessageBusBridge->Initialize(MessageBus.Get());
    DRIVE_LOG_UE(Log, TEXT("[DRIVE] MessageBus created"));

    // Managers (T2B)
    AgentManager = NewObject<UDriveAgentManager>(this);
    PossessionManager = NewObject<UDrivePossessionManager>(this);

    if (AgentManager)
    {
        AgentManager->Initialize(this);
    }
    if (PossessionManager)
    {
        PossessionManager->Initialize(this);
    }

    // Estado sesión (T2B)
    SetSimulationState(EDriveSimulationState::Uninitialized);
    LastError.Empty();

    #if WITH_EDITOR
    // Smoke Test T4
    MessageBusBridge->AddEventType(FDriveBusTestEvent::StaticStruct());
    FDriveBusTestEvent TestEvent;
    TestEvent.Value = 123;
    MessageBus->Publish(TestEvent);
    #endif
}

void UWorldSimulationSubsystem::Deinitialize()
{
    DRIVE_LOG_UE(Log, TEXT("[DRIVE] UWorldSimulationSubsystem Deinitialize"));
    StopSimulation();

    if (RuntimeStateChangedHandle.IsValid())
    {
        Runtime.OnRuntimeStateChanged().Remove(RuntimeStateChangedHandle);
        RuntimeStateChangedHandle.Reset();
    }

    if (ScenarioLoadedHandle.IsValid())
    {
        Runtime.OnScenarioLoaded().Remove(ScenarioLoadedHandle);
        ScenarioLoadedHandle.Reset();
    }

    if (TickHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
        TickHandle.Reset();
        DRIVE_LOG_UE(Warning, TEXT("[DRIVE] Ticker removed"));
    }

    if (this->MessageBusBridge)
    {
        this->MessageBusBridge->Shutdown();
        this->MessageBusBridge = nullptr;
    }

    Runtime.UnregisterAllSystems();
    SimulationHost.Reset();
    DRIVE_LOG_UE(Warning, TEXT("[DRIVE] SimulationHost destroyed"));
    MessageBus.Reset();
    DRIVE_LOG_UE(Log, TEXT("[DRIVE] MessageBus destroyed"));

    SetSimulationState(EDriveSimulationState::Uninitialized);
    AgentManager = nullptr;
    PossessionManager = nullptr;
    LastError.Empty();

    Super::Deinitialize();
    FDriveDebugCommands::Unregister();
}

bool UWorldSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    const UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;

    return (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UWorldSimulationSubsystem::InitializeSimulation(const UDriveGlobalConfigAsset* InGlobalConfig)
{
    CachedGlobalConfig = InGlobalConfig;

    float FixedDt = 1.0f / 60.0f;
    float TimeScale = 1.0f;
    int32 MaxSteps = 8;

    if (InGlobalConfig)
    {
        FixedDt = InGlobalConfig->FixedDeltaTime;
        TimeScale = InGlobalConfig->DefaultTimeScale;
        MaxSteps = InGlobalConfig->MaxFixedStepsPerFrame;
    }

    if (Runtime.IsInitialized())
    {
        UE_LOG(LogTemp, Warning, TEXT("[DRIVE] InitializeSimulation called again -> forcing clean reset"));
        StopSimulation();
        Runtime.Reset();
    }

    Clock.Initialize(FixedDt, TimeScale, MaxSteps);

    DRIVE_LOG_UE(Log, TEXT("[DRIVE] InitializeSimulation -> Clock init FixedDt=%.6f TimeScale=%.2f MaxSteps=%d"), Clock.GetFixedDeltaTimeSeconds(), Clock.GetTimeScale(), Clock.GetMaxFixedStepsPerFrame());
    if (!MessageBus.IsValid())
    {
        const FString Err = TEXT("InitializeSimulation failed: MessageBus is null");
        DRIVE_LOG_UE(Error, TEXT("[DRIVE] %s"), *Err);
        SetSimulationState(EDriveSimulationState::Error, Err);
        return;
    }
    
    Runtime.Initialize(&Clock, MessageBus.Get());

    RuntimeStateChangedHandle = Runtime.OnRuntimeStateChanged().AddUObject(this, &UWorldSimulationSubsystem::HandleRuntimeStateChanged);
    ScenarioLoadedHandle = Runtime.OnScenarioLoaded().AddUObject(this, &UWorldSimulationSubsystem::HandleScenarioLoaded);

    SystemContext = FDriveSystemContext{};
    SystemContext.GlobalConfig = InGlobalConfig;
    SystemContext.Clock = &Clock;
    SystemContext.Bus = MessageBus.Get();
    SystemContext.Runtime = &Runtime;
    SystemContext.SimState = &Runtime.GetSimState();
    SystemContext.EcsWorld = Runtime.GetEcsWorld();

    UE_LOG(LogDriveUE, Log, TEXT("[DRIVE] SystemContext valid=%d (Clock=%p Runtime=%p Bus=%p EcsWorld=%p)"),
        SystemContext.IsValid() ? 1 : 0,
        SystemContext.Clock,
        SystemContext.Runtime,
        SystemContext.Bus,
        SystemContext.EcsWorld
    );

	bIsRunning = false;
    DRIVE_LOG_UE(Log, TEXT("[DRIVE] InitializeSimulation -> Runtime ready (not running)"));

    LastError.Empty();
    SetSimulationState(EDriveSimulationState::Stopped);

    // Re-init managers (por seguridad en PIE)
    if (AgentManager) { AgentManager->Initialize(this); }
    if (PossessionManager) { PossessionManager->Initialize(this); }
}

void UWorldSimulationSubsystem::StartSimulation()
{
    FString Error;
    if (!Runtime.CanStart(Error))
    {
        SetSimulationState(EDriveSimulationState::Error, Error);
        DRIVE_LOG_UE(Error, TEXT("[DRIVE] StartSimulation blocked: %s"), *Error);
        return;
    }

    if (!Runtime.IsInitialized())
    {
        DRIVE_LOG_UE(Error, TEXT("[DRIVE] StartSimulation: Runtime not initialized"));
        return;
    }

    if (Runtime.GetState() == EScenarioRuntimeState::Running)
    {
        DRIVE_LOG_UE(Verbose, TEXT("[DRIVE] StartSimulation: already running"));
        bIsRunning = true;
        return;
    }

    //bIsRunning = true;
    Runtime.Start();
    bIsRunning = (Runtime.GetState() == EScenarioRuntimeState::Running);

    if (bIsRunning)
    {
        LastError.Empty();
        SetSimulationState(EDriveSimulationState::Running);
        OnSimulationStarted.Broadcast();
    }
    else
    {
        SetSimulationState(EDriveSimulationState::Error, TEXT("StartSimulation failed: runtime did not enter Running"));
    }

    if (bIsRunning && !TickHandle.IsValid())
    {
        TickHandle = FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateUObject(this, &UWorldSimulationSubsystem::Tick)
        );
        DRIVE_LOG_UE(Log, TEXT("[DRIVE] Subsystem ticker registered"));
    }


    DRIVE_LOG_UE(Log, TEXT("[DRIVE] StartSimulation -> bIsRunning=%d State=%d"),bIsRunning ? 1 : 0, (int32)Runtime.GetState());

    if (MessageBus && bIsRunning)
    {
        FDriveEvent_SimulationStarted Ev;
        Ev.ScenarioId = NAME_None;
        Ev.StartTimeSeconds = Clock.GetSimTimeSeconds();
        MessageBus->Publish(Ev);
    }
}

void UWorldSimulationSubsystem::PauseSimulation()
{
    OnSimulationPaused.Broadcast();
    SetSimulationState(EDriveSimulationState::Paused);
    bIsRunning = false;
    Runtime.Pause();
    DRIVE_LOG_UE(Log, TEXT("[DRIVE] PauseSimulation"));
}

void UWorldSimulationSubsystem::StopSimulation()
{
	if (!bIsRunning && Runtime.GetState() == EScenarioRuntimeState::Stopped)
        return;

    DRIVE_LOG_UE(Log, TEXT("[DRIVE][Determinism] FixedSteps=%llu SimTime=%.6f RealTime=%.6f Acc=%.6f ExecHash=0x%08x"),
        Runtime.GetFixedStepCount(), Clock.GetSimTimeSeconds(), Clock.GetRealTimeSeconds(), Clock.GetAccumulatedRealSeconds(), Runtime.GetExecutionOrderHash());

    bIsRunning = false;
    Runtime.Stop();

    if (TickHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
        TickHandle.Reset();
        DRIVE_LOG_UE(Log, TEXT("[DRIVE] Subsystem ticker removed"));
    }

    if (MessageBus)
    {
        FDriveEvent_SimulationStopped Ev;
        Ev.StopTimeSeconds = Clock.GetSimTimeSeconds();
        MessageBus->Publish(Ev);
    }
    OnSimulationStopped.Broadcast();
    SetSimulationState(EDriveSimulationState::Stopped);
    DRIVE_LOG_UE(Log, TEXT("[DRIVE] StopSimulation"));
}

void UWorldSimulationSubsystem::ResumeSimulation()
{
    Runtime.Resume();
    if (Runtime.GetState() == EScenarioRuntimeState::Running)
    {
        LastError.Empty();
        SetSimulationState(EDriveSimulationState::Running);
    }
    else
    {
        SetSimulationState(EDriveSimulationState::Error, TEXT("Resume failed: runtime did not return to Running"));
    }
    OnSimulationResumed.Broadcast();
    bIsRunning = (Runtime.GetState() == EScenarioRuntimeState::Running);
    DRIVE_LOG_UE(Log, TEXT("[DRIVE] ResumeSimulation -> bIsRunning=%d State=%d"), bIsRunning ? 1 : 0, (int32)Runtime.GetState());
}

int32 UWorldSimulationSubsystem::AdvanceClock(float DeltaSeconds)
{
    if (!bIsRunning)
    {
        return 0;
    }

    const int32 Steps = Runtime.Advance(DeltaSeconds);

    DRIVE_LOG_UE(Log, TEXT("[DRIVE][Clock] real=%.2f sim=%.2f acc=%.3f"), Clock.GetRealTimeSeconds(), Clock.GetSimTimeSeconds(), Clock.GetAccumulatedRealSeconds());
    DRIVE_LOG_UE(VeryVerbose, TEXT("[DRIVE] AdvanceClock dt=%.4f -> PendingSteps=%d AccAlpha=%.2f SimTime=%.3f"), DeltaSeconds, Steps, Clock.GetInterpolationAlpha(), Clock.GetSimTimeSeconds());
    return Steps;
}

bool UWorldSimulationSubsystem::LoadScenario(const UDriveScenarioAsset* ScenarioAsset)
{
    CachedScenarioAsset = ScenarioAsset;
    if (!Runtime.IsInitialized())
    {
        return Fail(TEXT("LoadScenario failed: Runtime not initialized (call InitializeSimulation first)"));
    }

    if (bIsRunning || Runtime.IsRunning())
    {
        return Fail(TEXT("LoadScenario failed: Simulation is running (stop before loading a new scenario)"));
    }

    if (Runtime.GetState() != EScenarioRuntimeState::Stopped)
    {
        return Fail(FString::Printf(TEXT("LoadScenario rejected: runtime not stopped (state=%d)"), (int32)Runtime.GetState()));
    }

	if (!ScenarioAsset)
	{
        return Fail(TEXT("LoadScenario failed: ScenarioAsset is null"));
	}

	FString Error;
	if (!ScenarioAsset->Validate(Error))
	{
        DRIVE_LOG_UE(Error, TEXT("[DRIVE] ScenarioAsset validation failed: %s"), *Error);
		return false;
	}

	const FDriveScenarioDefinition Def = ScenarioAsset->ToDefinition();
	if (!Runtime.LoadScenarioDefinition(Def))
	{
        DRIVE_LOG_UE(Error, TEXT("[DRIVE] Runtime rejected scenario definition"));
		return false;
	}

	Runtime.UnregisterAllSystems();

	for (const FName& SystemId : Def.Systems.EnabledSystems)
    {
        if (SystemId == TEXT("TestLogger"))
        {
            Runtime.RegisterSystem(MakeShared<FDriveSystem_TestLogger>());
        }
        else if (SystemId == TEXT("DummySystem"))
        {
            Runtime.RegisterSystem(MakeShared<FDriveDummySystem>());
        }
        else if (SystemId == TEXT("Drive.Movement"))
        {
            Runtime.RegisterSystem(MakeShared<FDriveMovementSystem>());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[DRIVE] Unknown SystemId in scenario: %s"), *SystemId.ToString());
            return false;
        }
    }

    SystemContext.GlobalConfig = CachedGlobalConfig;
    SystemContext.Clock = &Clock;
    SystemContext.Bus = MessageBus.Get();
    SystemContext.Runtime = &Runtime;
    SystemContext.SimState = &Runtime.GetSimState();
    SystemContext.EcsWorld = Runtime.GetEcsWorld();

    if (!SystemContext.IsValid())
    {
        UE_LOG(LogDriveUE, Error, TEXT("[DRIVE] LoadScenario failed: SystemContext invalid (did you call InitializeSimulation?)"));
        return false;
    }

    const bool bOk = Runtime.BuildExecutionPlanAndInitializeSystems(SystemContext);
    if (!bOk)
    {
        UE_LOG(LogDriveUE, Error, TEXT("[DRIVE] BuildExecutionPlan after scenario -> FAIL"));
        return false;
    }
    
    LastError.Empty();
    SetSimulationState(EDriveSimulationState::Ready);

    DRIVE_LOG_UE(Log, TEXT("[DRIVE] BuildExecutionPlan after scenario -> %s"), bOk ? TEXT("OK") : TEXT("FAIL"));
	return bOk;
}

void UWorldSimulationSubsystem::Advance(float DeltaSeconds)
{
    DRIVE_LOG_UE(Verbose, TEXT("[DRIVE] Advance() dt=%.4f bIsRunning=%d State=%d"), DeltaSeconds, bIsRunning ? 1 : 0, (int32)Runtime.GetState());
    if (!bIsRunning) return;

    const int32 Steps = Runtime.Advance(DeltaSeconds);
    if (Steps > 0)
    {
        OnFixedStepAdvanced.Broadcast((int64)Runtime.GetFixedStepCount(), (float)Clock.GetSimTimeSeconds());
    }

    if (Runtime.GetState() == EScenarioRuntimeState::Running)
    {
        const double Now = FPlatformTime::Seconds();
        if (Now >= GDriveNextSnapshotLogTime)
        {
            GDriveNextSnapshotLogTime = Now + 1.0; 

            const FDriveDebugSnapshot S = GetDebugSnapshot();

            UE_LOG(LogDriveUE, Warning,
                TEXT("[DRIVE][Snapshot] Sim=%.3f Real=%.3f Acc=%.3f Pending=%d Raw=%d Exec=%d Drop=%d | State=%d | RegSys=%d Plan=%d | BusQ=%d Types=%d Subs=%d | Hash=0x%08x"),
                S.SimTimeSeconds,
                S.RealTimeSeconds,
                S.AccumulatorSeconds,
                S.PendingFixedSteps,
                S.LastRawSteps,
                S.LastStepsExecuted,
                S.LastDroppedSteps,
                (int32)S.RuntimeState,
                S.RegisteredSystems,
                S.ExecutionPlanSystems,
                S.QueuedEvents,
                S.SubscriberEventTypes,
                S.TotalSubscribers,
                S.ExecutionOrderHash
            );
        }
    }
}

void UWorldSimulationSubsystem::RegisterSystem(TSharedRef<FDriveSystemBase> System)
{
    Runtime.RegisterSystem(System);
}

void UWorldSimulationSubsystem::UnregisterAllSystems()
{
    Runtime.UnregisterAllSystems();
}

void UWorldSimulationSubsystem::SetSimulationTimeScale(float InTimeScale)
{
    Clock.SetTimeScale(InTimeScale); 
}

float UWorldSimulationSubsystem::GetSimulationTimeScale() const
{
    return Clock.GetTimeScale();
}

FDriveDebugSnapshot UWorldSimulationSubsystem::GetDebugSnapshot() const
{
	return FDriveDebugSnapshot::Build(Clock, Runtime, MessageBus.Get());
}

EDriveRuntimeState UWorldSimulationSubsystem::ToDriveRuntimeState(EScenarioRuntimeState In) const
{
    switch (In)
    {
    case EScenarioRuntimeState::Running: return EDriveRuntimeState::Running;
    case EScenarioRuntimeState::Paused:  return EDriveRuntimeState::Paused;
    default:                              return EDriveRuntimeState::Stopped;
    }
}

void UWorldSimulationSubsystem::HandleRuntimeStateChanged(EScenarioRuntimeState OldState, EScenarioRuntimeState NewState)
{
    bIsRunning = (NewState == EScenarioRuntimeState::Running);
    OnRuntimeStateChanged.Broadcast(ToDriveRuntimeState(OldState), ToDriveRuntimeState(NewState));

    if (SimulationState != EDriveSimulationState::Error && SimulationState != EDriveSimulationState::Transitioning)
    {
        if (NewState == EScenarioRuntimeState::Running) SetSimulationState(EDriveSimulationState::Running);
        else if (NewState == EScenarioRuntimeState::Paused) SetSimulationState(EDriveSimulationState::Paused);
        else if (NewState == EScenarioRuntimeState::Stopped) SetSimulationState(EDriveSimulationState::Stopped);
    }
}

void UWorldSimulationSubsystem::HandleScenarioLoaded(FName ScenarioId)
{
    OnScenarioLoaded.Broadcast(ScenarioId);
}


// --------------------------------------------------------
//                          BP API 
// --------------------------------------------------------

// Simulation
void UWorldSimulationSubsystem::BP_InitializeSimulation(const UDriveGlobalConfigAsset* InGlobalConfig)
{
	if (bIsRunning || Runtime.IsRunning())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DRIVE][BP] Initialize blocked: simulation is running"));
		return;
	}

	InitializeSimulation(InGlobalConfig);
}

void UWorldSimulationSubsystem::BP_StartSimulation()
{
	StartSimulation();
}

void UWorldSimulationSubsystem::BP_PauseSimulation()
{
	if (!Runtime.IsRunning())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[DRIVE][BP] Pause ignored: not running"));
		return;
	}

	PauseSimulation();
}

void UWorldSimulationSubsystem::BP_ResumeSimulation()
{
	if (!Runtime.IsPaused())
	{
		UE_LOG(LogTemp, Verbose, TEXT("[DRIVE][BP] Resume ignored: runtime is not paused"));
		return;
	}

	ResumeSimulation();

	UE_LOG(LogTemp, Warning, TEXT("[DRIVE][BP] ResumeSimulation"));
}

void UWorldSimulationSubsystem::BP_StopSimulation()
{
	StopSimulation();
}

EDriveSimulationState UWorldSimulationSubsystem::BP_GetSimulationState() const
{
	return SimulationState;
}

bool UWorldSimulationSubsystem::BP_LoadScenario(const UDriveScenarioAsset* ScenarioAsset)
{
    const bool bOk = LoadScenario(ScenarioAsset);
    UE_LOG(LogTemp, Warning, TEXT("[DRIVE][BP] LoadScenario -> %s"), bOk ? TEXT("OK") : TEXT("FAIL"));
    return bOk;
}

bool UWorldSimulationSubsystem::Tick(float DeltaSeconds)
{
    if (!bIsRunning)
    {
        return true;
    }

    Advance(DeltaSeconds);
    return true;
}

// Runtime
bool UWorldSimulationSubsystem::BP_IsRuntimeInitialized() const
{
    return Runtime.IsInitialized();
}

EDriveRuntimeState UWorldSimulationSubsystem::BP_GetRuntimeState() const
{
    switch (Runtime.GetState())
    {
        case EScenarioRuntimeState::Running: return EDriveRuntimeState::Running;
        case EScenarioRuntimeState::Paused:  return EDriveRuntimeState::Paused;
        case EScenarioRuntimeState::Stopped:
        default:                             return EDriveRuntimeState::Stopped;
    }
}

bool UWorldSimulationSubsystem::BP_HasScenarioLoaded() const
{
    return Runtime.HasScenarioLoaded();
}

bool UWorldSimulationSubsystem::BP_IsScenarioLocked() const
{
    return Runtime.IsScenarioLocked();
}

int32 UWorldSimulationSubsystem::BP_GetActiveAgentCount() const
{
    return Runtime.GetActiveAgentCount();
}

int64 UWorldSimulationSubsystem::BP_GetRuntimeFixedStepCount() const
{
    return static_cast<int64>(Runtime.GetFixedStepCount());
}

int32 UWorldSimulationSubsystem::BP_GetExecutionOrderHash() const
{
    return static_cast<int32>(Runtime.GetExecutionOrderHash());
}

// Clock
float UWorldSimulationSubsystem::BP_GetClockRealTimeSeconds() const
{
    return static_cast<float>(Clock.GetRealTimeSeconds());
}

float UWorldSimulationSubsystem::BP_GetClockSimTimeSeconds() const
{
    return static_cast<float>(Clock.GetSimTimeSeconds());
}

float UWorldSimulationSubsystem::BP_GetClockFixedDeltaTimeSeconds() const
{
    return Clock.GetFixedDeltaTimeSeconds();
}

float UWorldSimulationSubsystem::BP_GetClockTimeScale() const
{
    return Clock.GetTimeScale();
}

int32 UWorldSimulationSubsystem::BP_GetClockMaxFixedStepsPerFrame() const
{
    return Clock.GetMaxFixedStepsPerFrame();
}

int32 UWorldSimulationSubsystem::BP_GetClockPendingFixedSteps() const
{
    return Clock.GetPendingFixedSteps();
}

int64 UWorldSimulationSubsystem::BP_GetClockFixedStepCount() const
{
    return Clock.GetFixedStepCount();
}

float UWorldSimulationSubsystem::BP_GetClockAccumulatorSeconds() const
{
    return static_cast<float>(Clock.GetAccumulatorSeconds());
}

float UWorldSimulationSubsystem::BP_GetClockInterpolationAlpha() const
{
    return Clock.GetInterpolationAlpha();
}

float UWorldSimulationSubsystem::BP_GetClockLastRealDeltaSeconds() const
{
    return Clock.GetLastRealDeltaSeconds();
}

float UWorldSimulationSubsystem::BP_GetClockLastSimDeltaSeconds() const
{
    return Clock.GetLastSimDeltaSeconds();
}

float UWorldSimulationSubsystem::BP_GetClockLastScaledDeltaSeconds() const
{
    return static_cast<float>(Clock.GetLastScaledDelta());
}

int32 UWorldSimulationSubsystem::BP_GetClockLastRawSteps() const
{
    return Clock.GetLastRawSteps();
}

int32 UWorldSimulationSubsystem::BP_GetClockLastStepsExecuted() const
{
    return Clock.GetLastStepsExecuted();
}

int32 UWorldSimulationSubsystem::BP_GetClockLastDroppedSteps() const
{
    return Clock.GetLastDroppedSteps();
}


int32 UWorldSimulationSubsystem::StepOnce(int32 NumSteps)
{
    if (NumSteps <= 0) return 0;

    if (SimulationState != EDriveSimulationState::Paused &&
        SimulationState != EDriveSimulationState::Ready)
    {
        return 0;
    }

    const int32 Done = Runtime.AdvanceFixedSteps_Manual(NumSteps);

    if (Done > 0)
    {
        OnFixedStepAdvanced.Broadcast((int64)Runtime.GetFixedStepCount(),
                                      (float)Clock.GetSimTimeSeconds());
    }

    return Done;
}

int32 UWorldSimulationSubsystem::BP_StepOnce(int32 NumSteps)
{
    return StepOnce(NumSteps);
}

bool UWorldSimulationSubsystem::ResetSimulation(bool bReloadScenario)
{
    // Stop seguro
    StopSimulation();

    // Reinicializar clock/runtime con config cacheada
    InitializeSimulation(CachedGlobalConfig);

    if (bReloadScenario && CachedScenarioAsset)
    {
        const bool bOk = LoadScenario(CachedScenarioAsset);
        return bOk;
    }

    return true;
}

bool UWorldSimulationSubsystem::BP_ResetSimulation(bool bReloadScenario)
{
    return ResetSimulation(bReloadScenario);
}

bool UWorldSimulationSubsystem::BP_TryStartSimulation()
{
    const EScenarioRuntimeState Before = Runtime.GetState();
    StartSimulation();
    return (Runtime.GetState() == EScenarioRuntimeState::Running) || (Before == EScenarioRuntimeState::Running);
}

bool UWorldSimulationSubsystem::BP_TryLoadScenario(const UDriveScenarioAsset* ScenarioAsset)
{
    return LoadScenario(ScenarioAsset);
}

void UWorldSimulationSubsystem::SetSimulationState(EDriveSimulationState NewState, const FString& Error)
{
    SimulationState = NewState;
    bIsRunning = (NewState == EDriveSimulationState::Running);

    if (!Error.IsEmpty())
    {
        LastError = Error;
    }
}

bool UWorldSimulationSubsystem::Fail(const FString& ErrorMsg)
{
    SetSimulationState(EDriveSimulationState::Error, ErrorMsg);
    DRIVE_LOG_UE(Error, TEXT("[DRIVE] %s"), *ErrorMsg);
    return false;
}