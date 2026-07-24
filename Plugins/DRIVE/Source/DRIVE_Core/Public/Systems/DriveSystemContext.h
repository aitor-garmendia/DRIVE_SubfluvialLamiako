#pragma once

#include "CoreMinimal.h"

class UDriveGlobalConfigAsset;
struct FSimulationClock;
class FDriveMessageBus;
class FScenarioRuntime;
class FDriveSimState;
class FDriveEcsWorld;

struct FDriveSystemContext
{
	const UDriveGlobalConfigAsset* GlobalConfig = nullptr;

	FSimulationClock* Clock = nullptr;
	FDriveMessageBus* Bus = nullptr;
	FScenarioRuntime* Runtime = nullptr;
	FDriveSimState* SimState = nullptr;
	FDriveEcsWorld* EcsWorld = nullptr;

	bool IsValid() const
	{
		return (Clock != nullptr) && (Runtime != nullptr) && (EcsWorld != nullptr) && (Bus != nullptr);
	}

	FDriveSystemContext() = default;

	FDriveSystemContext(FScenarioRuntime* InRuntime, 
		FSimulationClock* InClock, 
		FDriveMessageBus* InBus, 
		const UDriveGlobalConfigAsset* InGlobalConfig = nullptr)
		: GlobalConfig(InGlobalConfig), Clock(InClock), Bus(InBus), Runtime(InRuntime)
	{
	}

};
