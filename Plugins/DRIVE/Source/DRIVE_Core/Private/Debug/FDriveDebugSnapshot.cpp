#include "Debug/FDriveDebugSnapshot.h"

#include "Simulation/FSimulationClock.h"
#include "Messaging/FDriveMessageBus.h"
#include "Scenario/FScenarioRuntime.h"

FDriveDebugSnapshot FDriveDebugSnapshot::Build(const FSimulationClock& Clock, const FScenarioRuntime& Runtime, const FDriveMessageBus* Bus)
{
	FDriveDebugSnapshot S;

	// Clock 
	S.RealTimeSeconds = Clock.GetRealTimeSeconds();
	S.SimTimeSeconds = Clock.GetSimTimeSeconds();
	S.AccumulatorSeconds = Clock.GetAccumulatorSeconds();

	S.FixedDeltaTimeSeconds = Clock.GetFixedDeltaTimeSeconds();
	S.TimeScale = Clock.GetTimeScale();

	S.bClockPaused = Clock.IsPaused();
	S.ClockFixedStepCount = Clock.GetFixedStepCount();

	S.PendingFixedSteps = Clock.GetPendingFixedSteps();
	S.LastRawSteps = Clock.GetLastRawSteps();
	S.LastStepsExecuted = Clock.GetLastStepsExecuted();
	S.LastDroppedSteps = Clock.GetLastDroppedSteps();
	S.LastScaledDelta = Clock.GetLastScaledDelta();

	// Runtime
	S.bRuntimeInitialized = Runtime.IsInitialized();
	S.RuntimeState = Runtime.GetState();

	S.bScenarioLoaded = Runtime.HasScenarioLoaded();
	S.bScenarioLocked = Runtime.IsScenarioLocked();

	S.ActiveAgents = Runtime.GetActiveAgentCount();

	S.RuntimeFixedStepCount = Runtime.GetFixedStepCount();
	S.ExecutionOrderHash = Runtime.GetExecutionOrderHash();

	S.RegisteredSystems = Runtime.GetRegisteredSystemCount();
	S.ExecutionPlanSystems = Runtime.GetExecutionPlanCount();

	Runtime.GetRegisteredSystemIds(S.RegisteredSystemIds);
	Runtime.GetExecutionPlanSystemIds(S.ExecutionPlanSystemIds);

	// Bus
	S.bHasBus = (Bus != nullptr);
	if (Bus)
	{
		S.QueuedEvents = Bus->GetQueuedEventCount();
		S.SubscriberEventTypes = Bus->GetSubscriberEventTypeCount();
		S.TotalSubscribers = Bus->GetTotalSubscriberCount();
	}

	return S;
}
