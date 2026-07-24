#pragma once

#include "CoreMinimal.h"
#include "Scenario/DriveScenarioTypes.h"

struct DRIVE_CORE_API FDriveScenarioHeader
{
	FName ScenarioId = NAME_None;
	FGuid ScenarioGuid;
	FDriveScenarioVersion DefinitionVersion = 1;

	FString DisplayName;
	FString Author;
	FString Description;

	bool IsValidBasic() const
	{
		return !ScenarioId.IsNone() && DefinitionVersion > 0;
	}
};

struct DRIVE_CORE_API FDriveScenarioGlobalParam
{
	FName Key = NAME_None;
	FString Value;

	bool IsValidBasic() const
	{
		return !Key.IsNone();
	}
};

struct DRIVE_CORE_API FDriveScenarioGlobalConfig
{
	int32 RandomSeed = 0;
	TArray<FDriveScenarioGlobalParam> GlobalParams;

	bool IsValidBasic() const
	{
		for (const FDriveScenarioGlobalParam& P : GlobalParams)
		{
			if (!P.IsValidBasic())
			{
				return false;
			}
		}
		return true;
	}
};


struct DRIVE_CORE_API FDriveScenarioSystemsConfig
{
	TArray<FName> EnabledSystems;
};

struct DRIVE_CORE_API FDriveScenarioDefinition
{
	FDriveScenarioHeader Header;
	FDriveScenarioGlobalConfig Global;
	FDriveScenarioSystemsConfig Systems;
	TArray<FDriveScenarioEntityTemplate> EntityTemplates;
	TArray<FDriveScenarioSpawnSpec> Spawns;

	bool IsValidBasic() const
	{
		if (!Header.IsValidBasic())
		{
			return false;
		}

		if (!Global.IsValidBasic())
		{
			return false;
		}
		
		if (Systems.EnabledSystems.Num() == 0)
		{
			return false;
		}

		TSet<FName> SeenSystems;
		SeenSystems.Reserve(Systems.EnabledSystems.Num());

		for (const FName& Sys : Systems.EnabledSystems)
		{
			if (Sys.IsNone())
			{
				return false; 
			}

			if (SeenSystems.Contains(Sys))
			{
				return false;
			}
			SeenSystems.Add(Sys);
		}

		for (const FDriveScenarioEntityTemplate& Tpl : EntityTemplates)
		{
			if (!Tpl.IsValidBasic())
			{
				return false;
			}
		}

		for (const FDriveScenarioSpawnSpec& Spawn : Spawns)
		{
			if (!Spawn.IsValidBasic())
			{
				return false;
			}
		}

		return true;
	}
};