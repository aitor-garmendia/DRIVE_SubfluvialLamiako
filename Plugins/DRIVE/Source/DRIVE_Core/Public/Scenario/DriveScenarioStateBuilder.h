#pragma once

#include "CoreMinimal.h"
#include "Scenario/DriveScenarioDefinition.h"
#include "Scenario/DriveScenarioInitState.h"

class DRIVE_CORE_API FDriveScenarioStateBuilder
{
public:
	static bool BuildInitialState(const FDriveScenarioDefinition& Def, FDriveScenarioInitState& OutState);

private:
	static uint32 ComputeDeterministicSeed(const FDriveScenarioDefinition& Def);
};
