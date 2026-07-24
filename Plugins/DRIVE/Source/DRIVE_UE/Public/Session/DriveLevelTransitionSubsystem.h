#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/DriveSessionTypes.h"
#include "DriveLevelTransitionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDriveOnLevelLoadProgress, const FDriveLevelLoadProgress&, Progress);

UCLASS()
class DRIVE_UE_API UDriveLevelTransitionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="DRIVE|Levels")
    void RequestOpenLevel(const FName LevelName);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Levels")
    FDriveLevelLoadProgress GetProgress() const { return CurrentProgress; }

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Levels")
    FDriveOnLevelLoadProgress OnLevelLoadProgress;

private:
    void SetProgress(float P01, const FText& Phase, bool bActive);

    UPROPERTY()
    FDriveLevelLoadProgress CurrentProgress;
};