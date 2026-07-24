#pragma once

#include "CoreMinimal.h"
#include "DriveSessionTypes.generated.h"

UENUM(BlueprintType)
enum class EDriveSessionState : uint8
{
    Uninitialized UMETA(DisplayName="Uninitialized"),
    Ready         UMETA(DisplayName="Ready"),
    Running       UMETA(DisplayName="Running"),
    Paused        UMETA(DisplayName="Paused"),
    Stopped       UMETA(DisplayName="Stopped"),
    Error         UMETA(DisplayName="Error"),
    Transitioning UMETA(DisplayName="Transitioning")
};

UENUM(BlueprintType)
enum class EDriveAgentType : uint8
{
    Pedestrian UMETA(DisplayName="Pedestrian"),
    Vehicle    UMETA(DisplayName="Vehicle"),
    Drone      UMETA(DisplayName="Drone")
};

USTRUCT(BlueprintType)
struct FDriveAgentHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE")
    int32 Id = INDEX_NONE;

    bool IsValid() const { return Id != INDEX_NONE; }
};

USTRUCT(BlueprintType)
struct FDriveLevelLoadProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="DRIVE")
    float Progress01 = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="DRIVE")
    FText Phase;

    UPROPERTY(BlueprintReadOnly, Category="DRIVE")
    bool bIsActive = false;
};