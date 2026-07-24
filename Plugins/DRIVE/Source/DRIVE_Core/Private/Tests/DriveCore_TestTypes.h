#pragma once

#include "CoreMinimal.h"
#include "DriveCore_TestTypes.generated.h"

USTRUCT()
struct FDriveTestEvent_Int
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;
};
