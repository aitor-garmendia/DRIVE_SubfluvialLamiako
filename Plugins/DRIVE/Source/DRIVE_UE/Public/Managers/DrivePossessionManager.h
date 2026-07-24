#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "Types/DriveSessionTypes.h"

#include "DrivePossessionManager.generated.h"

class UWorldSimulationSubsystem;
class APlayerController;
class AActor;

USTRUCT(BlueprintType)
struct FDriveCameraTransitionSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE")
    float BlendTime = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE")
    TEnumAsByte<EViewTargetBlendFunction> BlendFunc = VTBlend_Cubic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE")
    float BlendExp = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE")
    bool bLockOutgoing = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDriveOnActiveViewTargetChanged, AActor*, NewTarget);

UCLASS(BlueprintType)
class DRIVE_UE_API UDrivePossessionManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UWorldSimulationSubsystem* InSubsystem);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Switching")
    bool SwitchViewTarget(APlayerController* PC, AActor* NewTarget, const FDriveCameraTransitionSettings& Settings);

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Switching")
    FDriveOnActiveViewTargetChanged OnActiveViewTargetChanged;

private:
    UPROPERTY()
    UWorldSimulationSubsystem* Subsystem = nullptr;
};