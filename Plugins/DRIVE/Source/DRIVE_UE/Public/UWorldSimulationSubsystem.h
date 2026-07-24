#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Simulation/FSimulationClock.h"
#include "Scenario/FScenarioRuntime.h"
#include "Messaging/FDriveMessageBus.h"
#include "Scenario/DriveScenarioAsset.h"
#include "Debug/FDriveDebugSnapshot.h"
#include "Systems/DriveSystemContext.h" 
#include "Messaging/DriveMessageBusBridge.h"
#include "FDriveSimulationHost.h"

#include "Containers/Ticker.h"
#include "Templates/UniquePtr.h"
#include "UWorldSimulationSubsystem.generated.h"


class UDriveGlobalConfigAsset;
class FDriveMessageBus;
class DriveSystemBase;
class UDriveAgentManager;
class UDrivePossessionManager;

UENUM(BlueprintType)
enum class EDriveSimulationState : uint8
{
    Uninitialized  UMETA(DisplayName="Uninitialized"),
    Ready          UMETA(DisplayName="Ready"),
    Running        UMETA(DisplayName="Running"),
    Paused         UMETA(DisplayName="Paused"),
    Stopped        UMETA(DisplayName="Stopped"),
    Transitioning  UMETA(DisplayName="Transitioning"),
    Error          UMETA(DisplayName="Error")
};

UENUM(BlueprintType)
enum class EDriveRuntimeState : uint8
{
    Stopped UMETA(DisplayName="Stopped"),
    Running UMETA(DisplayName="Running"),
    Paused  UMETA(DisplayName="Paused")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDriveOnSimulationStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDriveOnSimulationPaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDriveOnSimulationResumed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDriveOnSimulationStopped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDriveOnScenarioLoaded, FName, ScenarioId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDriveOnRuntimeStateChanged, EDriveRuntimeState, OldState, EDriveRuntimeState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDriveOnFixedStepAdvanced, int64, FixedStepCount, float, SimTimeSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDriveOnLevelTransitionChanged, bool, bIsTransitioning);


UCLASS()
class DRIVE_UE_API UWorldSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // --------------------------------------------------------
    //                            BP 
    // --------------------------------------------------------

    // Simulation
	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
	void BP_InitializeSimulation(const UDriveGlobalConfigAsset* InGlobalConfig);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
    bool BP_LoadScenario(const UDriveScenarioAsset* ScenarioAsset);

	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
	void BP_StartSimulation();

	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
	void BP_PauseSimulation();

	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
	void BP_ResumeSimulation();

	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
	void BP_StopSimulation();

	UFUNCTION(BlueprintPure, Category="DRIVE|Simulation")
	EDriveSimulationState BP_GetSimulationState() const;

    UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
    bool BP_TryStartSimulation();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
    bool BP_TryLoadScenario(const UDriveScenarioAsset* ScenarioAsset);

    //  Runtime state
    UFUNCTION(BlueprintPure, Category="DRIVE|Runtime")
    bool BP_IsRuntimeInitialized() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Runtime")
    EDriveRuntimeState BP_GetRuntimeState() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Runtime")
    bool BP_HasScenarioLoaded() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Runtime")
    bool BP_IsScenarioLocked() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Runtime")
    int32 BP_GetActiveAgentCount() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Runtime")
    int64 BP_GetRuntimeFixedStepCount() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Runtime")
    int32 BP_GetExecutionOrderHash() const;

    
    // Clock
    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    float BP_GetClockRealTimeSeconds() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    float BP_GetClockSimTimeSeconds() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    float BP_GetClockFixedDeltaTimeSeconds() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    float BP_GetClockTimeScale() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    int32 BP_GetClockMaxFixedStepsPerFrame() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    int32 BP_GetClockPendingFixedSteps() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    int64 BP_GetClockFixedStepCount() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    float BP_GetClockAccumulatorSeconds() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    float BP_GetClockInterpolationAlpha() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    float BP_GetClockLastRealDeltaSeconds() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    float BP_GetClockLastSimDeltaSeconds() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    float BP_GetClockLastScaledDeltaSeconds() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    int32 BP_GetClockLastRawSteps() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    int32 BP_GetClockLastStepsExecuted() const;

    UFUNCTION(BlueprintPure, Category="DRIVE|Clock")
    int32 BP_GetClockLastDroppedSteps() const;

    UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
    int32 BP_StepOnce(int32 NumSteps = 1);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation")
    bool BP_ResetSimulation(bool bReloadScenario = true);


    // Blueprint events
    UPROPERTY(BlueprintAssignable, Category="DRIVE|Simulation")
    FDriveOnSimulationStarted OnSimulationStarted;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Simulation")
    FDriveOnSimulationPaused OnSimulationPaused;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Simulation")
    FDriveOnSimulationResumed OnSimulationResumed;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Simulation")
    FDriveOnSimulationStopped OnSimulationStopped;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Simulation")
    FDriveOnScenarioLoaded OnScenarioLoaded;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Simulation")
    FDriveOnRuntimeStateChanged OnRuntimeStateChanged;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Clock")
    FDriveOnFixedStepAdvanced OnFixedStepAdvanced;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Levels")
    FDriveOnLevelTransitionChanged OnLevelTransitionChanged;

    // LastError
    UFUNCTION(BlueprintPure, Category="DRIVE|Simulation")
    FString BP_GetLastError() const { return LastError; }

    // getters manager
    UFUNCTION(BlueprintPure, Category="DRIVE|Managers")
    UDriveAgentManager* BP_GetAgentManager() const { return AgentManager; }

    UFUNCTION(BlueprintPure, Category="DRIVE|Managers")
    UDrivePossessionManager* BP_GetPossessionManager() const { return PossessionManager; }


    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    void InitializeSimulation(const UDriveGlobalConfigAsset* InGlobalConfig);
    void StartSimulation();
    void PauseSimulation();
    void StopSimulation();
    void ResumeSimulation();

    bool IsRunning() const { return SimulationState == EDriveSimulationState::Running; }

    int32 AdvanceClock(float DeltaSeconds);
    const FSimulationClock& GetClock() const { return Clock; }
    FSimulationClock& GetClock() { return Clock; }
    int32 StepOnce(int32 NumSteps = 1);
    bool ResetSimulation(bool bReloadScenario = true);

    FDriveMessageBus* GetMessageBus() const { return MessageBus.Get(); }

    bool LoadScenario(const UDriveScenarioAsset* ScenarioAsset);
    void Advance(float DeltaSeconds);

    FScenarioRuntime& GetRuntime() { return Runtime; }
	const FScenarioRuntime& GetRuntime() const { return Runtime; }

	void RegisterSystem(TSharedRef<FDriveSystemBase> System);
	void UnregisterAllSystems();

    void SetSimulationTimeScale(float InTimeScale);
    float GetSimulationTimeScale() const;

    FDriveDebugSnapshot GetDebugSnapshot() const;
    UDriveMessageBusBridge* GetMessageBusBridge() const { return MessageBusBridge; }

private:
    bool bIsRunning = false;
    FSimulationClock Clock;
    FScenarioRuntime Runtime;
    FTSTicker::FDelegateHandle TickHandle;
    
    TUniquePtr<FDriveMessageBus> MessageBus;
    UPROPERTY(Transient)
    TObjectPtr<UDriveMessageBusBridge> MessageBusBridge = nullptr;

    const UDriveGlobalConfigAsset* CachedGlobalConfig = nullptr;

    TUniquePtr<FDriveSimulationHost> SimulationHost;
    FDriveSystemContext SystemContext;
    bool Tick(float DeltaSeconds);


    void HandleRuntimeStateChanged(EScenarioRuntimeState OldState, EScenarioRuntimeState NewState);
    void HandleScenarioLoaded(FName ScenarioId);

    EDriveRuntimeState ToDriveRuntimeState(EScenarioRuntimeState In) const;
    FDelegateHandle RuntimeStateChangedHandle;
    FDelegateHandle ScenarioLoadedHandle;

    FString LastError;
    UPROPERTY(Transient)
    TObjectPtr<UDriveAgentManager> AgentManager = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UDrivePossessionManager> PossessionManager = nullptr;

    EDriveSimulationState SimulationState = EDriveSimulationState::Uninitialized;
    void SetSimulationState(EDriveSimulationState NewState, const FString& Error = FString());

    const UDriveScenarioAsset* CachedScenarioAsset = nullptr;
    bool Fail(const FString& ErrorMsg);
};