#pragma once

#include "CoreMinimal.h"
#include "Scenario/DriveScenarioDefinition.h"

struct FDriveScenarioInitState
{
	FName ScenarioId = NAME_None;
	uint32 Seed = 0;

	TArray<FName> EnabledSystemsSorted;
	TArray<FDriveScenarioGlobalParam> GlobalParamsSorted;

	TArray<FDriveScenarioEntityTemplate> EntityTemplates;
	TArray<FDriveScenarioSpawnSpec> Spawns;

	bool IsValidBasic() const
	{
		return !ScenarioId.IsNone() && Seed != 0;
	}
};
