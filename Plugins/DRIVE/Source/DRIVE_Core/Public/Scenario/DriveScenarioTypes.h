#pragma once

#include "CoreMinimal.h"

using FDriveScenarioVersion = int32;

struct DRIVE_CORE_API FDriveScenarioParam
{
	FName Key = NAME_None;
	FString Value;

	bool IsSet() const
	{
		return !Key.IsNone();
	}
};

// Entity template
struct DRIVE_CORE_API FDriveScenarioEntityTemplate
{
	FName TemplateId = NAME_None;
	TMap<FName, FString> Properties;

	bool IsValidBasic() const
	{
		return !TemplateId.IsNone();
	}
};

struct DRIVE_CORE_API FDriveScenarioSpawnSpec
{
	FName TemplateId = NAME_None;
	int32 Count = 0;
	TArray<FTransform> Transforms;

	bool IsValidBasic() const
	{
		return !TemplateId.IsNone() && Count >= 0;
	}
};