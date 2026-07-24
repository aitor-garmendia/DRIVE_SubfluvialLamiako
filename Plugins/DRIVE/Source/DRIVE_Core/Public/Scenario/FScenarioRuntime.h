#pragma once

#include "CoreMinimal.h"
#include "Systems/DriveSystemContext.h" 
#include "Scenario/DriveScenarioDefinition.h"
#include "Systems/DriveSystemBase.h"
#include "Simulation/FDriveSimState.h"
#include "Scenario/DriveScenarioLoadResult.h"
#include "Scenario/DriveScenarioInitState.h"

struct FSimulationClock;
class FDriveMessageBus;
class FDriveSystemBase;
struct FAgentBase;
class FDriveEcsWorld;

enum class EScenarioRuntimeState : uint8
{
    Stopped,
    Running,
    Paused
};

struct FScenarioRuntimeStats
{
	// Estado observable
	bool bInitialized = false;
	bool bScenarioLoaded = false;
	bool bScenarioLocked = false;
	EScenarioRuntimeState State = EScenarioRuntimeState::Stopped;

	// Systems
	int32 RegisteredSystems = 0;
	int32 ActiveFixedTickSystems = 0;

	// Ejecución
	uint64 FixedStepCount = 0;
	uint32 ExecutionOrderHash = 0;

	// Último Advance()
	float LastAdvanceRealDeltaSeconds = 0.0f;
	int32 LastAdvanceFixedStepsExecuted = 0;
	float LastAdvanceFixedDt = 0.0f;

	// Timing
	double LastFixedStepWallSeconds = 0.0;
	double LastAdvanceWallSeconds = 0.0;
	double TotalFixedStepWallSeconds = 0.0;

	// Validaciones
	uint64 InvalidStateTransitions = 0;
	uint64 InvalidAdvanceCalls = 0;
	uint64 SkippedSystemTicks = 0;

	// Perfilado del último fixed-step 
	TArray<double> LastSystemTickWallSeconds;
};

class DRIVE_CORE_API FScenarioRuntime
{
public:
    FScenarioRuntime();
    ~FScenarioRuntime();

    // Runtime internal events
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRuntimeStateChanged, EScenarioRuntimeState /*OldState*/, EScenarioRuntimeState /*NewState*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnScenarioLoaded, FName /*ScenarioId*/);

    FOnRuntimeStateChanged& OnRuntimeStateChanged() { return RuntimeStateChanged; }
    FOnScenarioLoaded& OnScenarioLoaded() { return ScenarioLoaded; }

    void Initialize(FSimulationClock* InClock, FDriveMessageBus* InBus);
    bool IsInitialized() const { return bInitialized; }

    void Start();
    void Pause();
    void Stop();
    void Reset();
    void Resume();
    bool CanStart(FString& OutError) const;
    
    EScenarioRuntimeState GetState() const { return State; }
    bool IsRunning() const { return State == EScenarioRuntimeState::Running; }
    bool IsPaused()  const { return State == EScenarioRuntimeState::Paused; }
    int32 Advance(float RealDeltaSeconds);

    void AddAgent(TSharedPtr<FAgentBase> Agent);
    void RemoveAgent(const FGuid& AgentId);
    int32 GetActiveAgentCount() const { return ActiveAgents.Num(); }

    FSimulationClock* GetClock() const { return Clock; }
    FDriveMessageBus* GetBus() const { return Bus; }

	void RegisterSystem(TSharedRef<FDriveSystemBase> System);
	void UnregisterAllSystems();
	bool BuildExecutionPlanAndInitializeSystems(const FDriveSystemContext& Context);
	void AdvanceFixedStep(float FixedDt);

    bool LoadScenarioDefinition(const FDriveScenarioDefinition& InDef);
	const FDriveScenarioDefinition& GetScenarioDefinition() const { return ScenarioDef; }
	bool HasScenarioLoaded() const { return bScenarioLoaded; }

    bool IsScenarioLocked() const { return bScenarioLocked; }
    void UnlockScenario();

    FDriveSimState& GetSimState() { return SimState; }
    const FDriveSimState& GetSimState() const { return SimState; }

    FDriveEcsWorld* GetEcsWorld() const { return EcsWorld.Get(); }

    const FDriveScenarioLoadResult& GetScenarioLoadResult() const { return ScenarioLoadResult; }

    bool BuildInitialStateFromScenario();
	bool HasInitialState() const { return bInitialStateBuilt; }
	const FDriveScenarioInitState& GetInitialState() const { return InitialState; }

    void ResetDeterminismStats();
    uint64 GetFixedStepCount() const { return FixedStepCount; }
    uint32 GetExecutionOrderHash() const { return ExecutionOrderHash; }

    // Observabilidad
	const FScenarioRuntimeStats& GetStats() const { return Stats; }
	void ResetRuntimeStats();
	void GetLastFixedTickSystemIds(TArray<FName>& OutIds) const;
	void GetLastFixedTickSystemTimes(TArray<double>& OutSeconds) const;

    void GetRegisteredSystemIds(TArray<FName>& OutIds) const;
    void GetExecutionPlanSystemIds(TArray<FName>& OutIds) const;

    int32 GetRegisteredSystemCount() const { return Systems.Num(); }
    int32 GetExecutionPlanCount() const { return ExecutionPlan_FixedTick.Num(); }
    bool HasSystemContext() const { return bHasSystemContext; }
    bool HasBus() const { return Bus != nullptr; }
    bool HasClock() const { return Clock != nullptr; }

    int32 AdvanceFixedSteps_Manual(int32 NumSteps);

private:
    void SetState(EScenarioRuntimeState NewState);
    bool TopoSortSystems(TArray<TSharedRef<FDriveSystemBase>>& OutSortedInit) const;

    bool bInitialized = false;
    EScenarioRuntimeState State = EScenarioRuntimeState::Stopped;
    void AdvanceFixedStep_Internal(float FixedDt);

    FSimulationClock* Clock = nullptr;
    FDriveMessageBus* Bus = nullptr;

    TArray<TSharedRef<FDriveSystemBase>> Systems;
    TArray<TSharedPtr<FAgentBase>> ActiveAgents;

    TMap<FName, TSharedRef<FDriveSystemBase>> SystemsById; 
	TArray<TSharedRef<FDriveSystemBase>> ExecutionPlan_FixedTick;

    bool bScenarioLoaded = false;
    bool bScenarioLocked = false; 
	FDriveScenarioDefinition ScenarioDef;

    bool bHasSystemContext = false;
    FDriveSystemContext SystemContext;

    FDriveSimState SimState;

    TUniquePtr<FDriveEcsWorld> EcsWorld;

    FDriveScenarioLoadResult ScenarioLoadResult;

    bool bInitialStateBuilt = false;
	FDriveScenarioInitState InitialState;

    uint64 FixedStepCount = 0;
    uint32 ExecutionOrderHash = 0;

    FScenarioRuntimeStats Stats;

    FOnRuntimeStateChanged RuntimeStateChanged;
    FOnScenarioLoaded ScenarioLoaded;
};