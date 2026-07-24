#pragma once
#include "CoreMinimal.h"
#include "DriveBusTestEvents.generated.h"

USTRUCT()
struct FDriveBusTestEvent
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;
};