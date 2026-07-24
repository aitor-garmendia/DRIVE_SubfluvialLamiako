#pragma once

#include "CoreMinimal.h"
#include "DriveEventBase.h"
#include "DriveSimulationEvents.generated.h"

USTRUCT(BlueprintType)
struct DRIVE_CORE_API FDriveEvent_SimulationStarted : public FDriveEventBase
{
	GENERATED_BODY()

	UPROPERTY() FName  ScenarioId = NAME_None;
	UPROPERTY() double StartTimeSeconds = 0.0;
};

USTRUCT(BlueprintType)
struct DRIVE_CORE_API FDriveEvent_SimulationStopped : public FDriveEventBase
{
	GENERATED_BODY()

	UPROPERTY() double StopTimeSeconds = 0.0;
};

USTRUCT(BlueprintType)
struct DRIVE_CORE_API FDriveEvent_FixedStep : public FDriveEventBase
{
	GENERATED_BODY()

	UPROPERTY() int32  StepIndexThisFrame = 0;
	UPROPERTY() float  FixedDt = 0.0f;
	UPROPERTY() double SimTimeSeconds = 0.0;
};
