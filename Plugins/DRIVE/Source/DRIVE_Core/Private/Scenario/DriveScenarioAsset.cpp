#include "Scenario/DriveScenarioAsset.h"

bool UDriveScenarioAsset::Validate(FString& OutError) const
{
	if (ScenarioId.IsNone())
	{
		OutError = TEXT("ScenarioId is None.");
		return false;
	}

	{
		TSet<FName> Seen;
		for (const FName& Id : EnabledSystems)
		{
			if (Id.IsNone())
			{
				OutError = TEXT("EnabledSystems contains None.");
				return false;
			}
			if (Seen.Contains(Id))
			{
				OutError = FString::Printf(TEXT("EnabledSystems contains duplicated id: %s"), *Id.ToString());
				return false;
			}
			Seen.Add(Id);
		}
	}

	return true;
}

FDriveScenarioDefinition UDriveScenarioAsset::ToDefinition() const
{
	FDriveScenarioDefinition Def;
	Def.Header.ScenarioId = ScenarioId;
	Def.Systems.EnabledSystems = EnabledSystems;
	return Def;
}
