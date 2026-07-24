#pragma once

#include "CoreMinimal.h"
#include "DriveEventBase.h"
#include "DriveBaseMessages.generated.h"

UENUM(BlueprintType)
enum class EDriveBusLogLevel : uint8
{
	Trace   UMETA(DisplayName="Trace"),
	Debug   UMETA(DisplayName="Debug"),
	Info    UMETA(DisplayName="Info"),
	Warning UMETA(DisplayName="Warning"),
	Error   UMETA(DisplayName="Error")
};

USTRUCT(BlueprintType)
struct DRIVE_CORE_API FDriveEvent_FixedTick : public FDriveEventBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float FixedDt = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	double SimTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadOnly)
	int64 StepIndex = 0;
};

USTRUCT(BlueprintType)
struct DRIVE_CORE_API FDriveEvent_RuntimeStateChanged : public FDriveEventBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	uint8 OldState = 0;

	UPROPERTY(BlueprintReadOnly)
	uint8 NewState = 0;
};

USTRUCT(BlueprintType)
struct DRIVE_CORE_API FDriveEvent_BusLog : public FDriveEventBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EDriveBusLogLevel Level = EDriveBusLogLevel::Info;

	UPROPERTY(BlueprintReadOnly)
	FName Source = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	FString Message;
};
