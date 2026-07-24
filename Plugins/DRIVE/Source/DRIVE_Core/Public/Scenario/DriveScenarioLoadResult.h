#pragma once

#include "CoreMinimal.h"

struct FDriveScenarioLoadResult
{
	FName ScenarioId = NAME_None;
	TArray<FName> RequestedSystems;
	TArray<FName> MissingSystems;
	bool bDefinitionValid = false;

	void Reset()
	{
		ScenarioId = NAME_None;
		RequestedSystems.Reset();
		MissingSystems.Reset();
		bDefinitionValid = false;
	}
};
