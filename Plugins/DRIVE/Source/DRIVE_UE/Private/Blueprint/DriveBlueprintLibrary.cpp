#include "Blueprint/DriveBlueprintLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

#include "UWorldSimulationSubsystem.h"
#include "Scenario/DriveScenarioAsset.h"
#include "DriveGlobalConfigAsset.h"
#include "Systems/FDriveDummySystem.h"
#include "Systems/FDriveMovementSystem.h"
#include "Systems/DriveSystemContext.h"
#include "Logging/DriveLog.h"

UWorldSimulationSubsystem* UDriveBlueprintLibrary::GetWorldSimulationSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}

	return World->GetSubsystem<UWorldSimulationSubsystem>();
}

bool UDriveBlueprintLibrary::IsDriveSimulationRunning(const UObject* WorldContextObject)
{
	if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject))
	{
		return Subsystem->IsRunning();
	}
	return false;
}

void UDriveBlueprintLibrary::DriveSim_Start(const UObject* WorldContextObject)
{
	if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject))
	{
		Subsystem->StartSimulation();
	}
}

void UDriveBlueprintLibrary::DriveSim_Pause(const UObject* WorldContextObject)
{
	if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject))
	{
		Subsystem->PauseSimulation();
	}
}

void UDriveBlueprintLibrary::DriveSim_Stop(const UObject* WorldContextObject)
{
	if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject))
	{
		Subsystem->StopSimulation();
	}
}

bool UDriveBlueprintLibrary::DriveSim_LoadScenario(const UObject* WorldContextObject, const UDriveScenarioAsset* ScenarioAsset)
{
	if (!ScenarioAsset)
	{
		return false;
	}

	if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject))
	{
		return Subsystem->LoadScenario(ScenarioAsset);
	}

	return false;
}

void UDriveBlueprintLibrary::DriveSim_SetTimeScale(const UObject* WorldContextObject, float TimeScale)
{
	if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject))
	{
		Subsystem->SetSimulationTimeScale(TimeScale);
	}
}

float UDriveBlueprintLibrary::DriveSim_GetTimeScale(const UObject* WorldContextObject)
{
	if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject))
	{
		return Subsystem->GetSimulationTimeScale();
	}
	return 1.0f;
}

bool UDriveBlueprintLibrary::DriveSim_InitializeAndBuildDefaults(const UObject* WorldContextObject)
{
	UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject);
	if (!Subsystem)
	{
		return false;
	}

	if (!Subsystem->GetRuntime().IsInitialized())
	{
		Subsystem->InitializeSimulation(nullptr);
	}

	Subsystem->UnregisterAllSystems();
	Subsystem->RegisterSystem(MakeShared<FDriveDummySystem>());
	Subsystem->RegisterSystem(MakeShared<FDriveMovementSystem>());

	FScenarioRuntime& Runtime = Subsystem->GetRuntime();
	FDriveSystemContext Ctx;
	Ctx.GlobalConfig = nullptr;
	Ctx.Clock = &Subsystem->GetClock();
	Ctx.Bus = Subsystem->GetMessageBus();
	Ctx.Runtime = &Runtime;
	Ctx.SimState = &Runtime.GetSimState();
	Ctx.EcsWorld = Runtime.GetEcsWorld();

	const bool bOk = Runtime.BuildExecutionPlanAndInitializeSystems(Ctx);
	if (!bOk)
	{
		DRIVE_LOG_DEBUG(Error, TEXT("[DRIVE] DriveSim_InitializeAndBuildDefaults: BuildExecutionPlan failed"));
	}

	return bOk;
}

bool UDriveBlueprintLibrary::DriveSim_Initialize(const UObject* WorldContextObject)
{
	if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject))
	{
		if (Subsystem->GetRuntime().IsInitialized())
		{
			return true;
		}

		Subsystem->InitializeSimulation(nullptr); 
		return Subsystem->GetRuntime().IsInitialized();
	}
	return false;
}

bool UDriveBlueprintLibrary::DriveSim_HasScenarioLoaded(const UObject* WorldContextObject)
{
	if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem(WorldContextObject))
	{
		return Subsystem->BP_HasScenarioLoaded();
	}
	return false;
}