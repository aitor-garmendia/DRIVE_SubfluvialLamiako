#pragma once

#include "CoreMinimal.h"
#include "DriveEventBase.h"
#include "DriveTestEvents.generated.h"

USTRUCT()
struct DRIVE_CORE_API FDriveEvent_TestPing : public FDriveEventBase
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;
};

USTRUCT()
struct DRIVE_CORE_API FDriveEvent_TestPong : public FDriveEventBase
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;
};