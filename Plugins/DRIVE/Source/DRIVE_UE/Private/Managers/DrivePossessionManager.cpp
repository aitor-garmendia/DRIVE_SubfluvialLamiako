#include "Managers/DrivePossessionManager.h"
#include "UWorldSimulationSubsystem.h"
#include "GameFramework/PlayerController.h"

void UDrivePossessionManager::Initialize(UWorldSimulationSubsystem* InSubsystem)
{
    Subsystem = InSubsystem;
}

bool UDrivePossessionManager::SwitchViewTarget(APlayerController* PC, AActor* NewTarget, const FDriveCameraTransitionSettings& Settings)
{
    if (!PC || !NewTarget) return false;

    PC->SetViewTargetWithBlend(NewTarget, Settings.BlendTime, Settings.BlendFunc, Settings.BlendExp, Settings.bLockOutgoing);
    OnActiveViewTargetChanged.Broadcast(NewTarget);
    return true;
}