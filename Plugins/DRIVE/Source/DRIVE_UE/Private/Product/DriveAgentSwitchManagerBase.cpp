#include "Product/DriveAgentSwitchManagerBase.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Logging/DriveLog.h"


ADriveAgentSwitchManagerBase::ADriveAgentSwitchManagerBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ADriveAgentSwitchManagerBase::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = ResolvePlayerController())
    {
        APawn* CurrentPawn = PC->GetPawn();
        ActivePawn = CurrentPawn;
    }
}

APlayerController* ADriveAgentSwitchManagerBase::ResolvePlayerController() const
{
    if (!bAutoFindPlayerController)
    {
        if (ExplicitPlayerController.IsValid())
        {
            return ExplicitPlayerController.Get();
        }

        if (!ExplicitPlayerController.IsNull())
        {
            return ExplicitPlayerController.LoadSynchronous();
        }
    }

    return UGameplayStatics::GetPlayerController(this, 0);
}

bool ADriveAgentSwitchManagerBase::SwitchToPawn(APawn* NewPawn)
{
    if (!NewPawn)
    {
        DRIVE_LOG_MANAGER(Log, TEXT("[DRIVE][SwitchManager] SwitchToPawn failed: NewPawn is null."));
        return false;
    }

    APlayerController* PC = ResolvePlayerController();
    if (!PC)
    {
        DRIVE_LOG_MANAGER(Error, TEXT("[DRIVE][SwitchManager] SwitchToPawn failed: PlayerController not found."));
        return false;
    }

    APawn* OldPawn = PC->GetPawn();

    if (OldPawn == NewPawn)
    {
        ActivePawn = NewPawn;
        return true;
    }

    if (CameraBlendTime > 0.f)
    {
        PC->SetViewTargetWithBlend(NewPawn, CameraBlendTime);
    }
    else
    {
        PC->SetViewTarget(NewPawn);
    }

    PC->Possess(NewPawn);

    ActivePawn = NewPawn;

    OnActivePawnChanged.Broadcast(OldPawn, NewPawn);

    DRIVE_LOG_MANAGER(Log, TEXT("[DRIVE][SwitchManager] Possessed: %s (Old: %s)"), *GetNameSafe(NewPawn), *GetNameSafe(OldPawn));

    return true;
}