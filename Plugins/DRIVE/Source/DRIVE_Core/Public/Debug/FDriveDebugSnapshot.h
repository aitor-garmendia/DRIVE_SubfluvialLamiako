#pragma once

#include "CoreMinimal.h"
#include "Scenario/FScenarioRuntime.h" 
#include "Debug/FDriveDebugSnapshot.h"

struct FSimulationClock;
class FDriveMessageBus;
class FScenarioRuntime;

struct DRIVE_CORE_API FDriveDebugSnapshot
{
	// Clock
	double RealTimeSeconds = 0.0;
	double SimTimeSeconds = 0.0;
	double AccumulatorSeconds = 0.0;

	float FixedDeltaTimeSeconds = 0.0f;
	float TimeScale = 1.0f;

	bool bClockPaused = false;

	uint64 ClockFixedStepCount = 0;

	int32 PendingFixedSteps = 0;
	int32 LastRawSteps = 0;
	int32 LastStepsExecuted = 0;
	int32 LastDroppedSteps = 0;
	double LastScaledDelta = 0.0;

	// Runtime
	bool bRuntimeInitialized = false;
	EScenarioRuntimeState RuntimeState = EScenarioRuntimeState::Stopped;

	bool bScenarioLoaded = false;
	bool bScenarioLocked = false;

	int32 ActiveAgents = 0;

	uint64 RuntimeFixedStepCount = 0;
	uint32 ExecutionOrderHash = 0;

	int32 RegisteredSystems = 0;
	int32 ExecutionPlanSystems = 0;

	TArray<FName> RegisteredSystemIds;
	TArray<FName> ExecutionPlanSystemIds;

	// MessageBus 
	bool bHasBus = false;
	int32 QueuedEvents = 0;
	int32 SubscriberEventTypes = 0;
	int32 TotalSubscribers = 0;
    
	static FDriveDebugSnapshot Build(const FSimulationClock& Clock, const FScenarioRuntime& Runtime, const FDriveMessageBus* Bus);
};
