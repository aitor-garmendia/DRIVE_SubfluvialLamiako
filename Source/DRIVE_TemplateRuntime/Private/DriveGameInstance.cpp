#include "DriveGameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Misc/CoreDelegates.h"    
#include "Logging/DriveLog.h"

void UDriveGameInstance::Init()
{
    Super::Init();
    DRIVE_LOG_UE(Log, TEXT("[DRIVE] GameInstance Init"));

    DriveHost = MakeUnique<FDriveSimulationHost>();
    PostWorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(
        this, &UDriveGameInstance::HandlePostWorldInit
    );
}

void UDriveGameInstance::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
    if (!World || !DriveHost)
    {
        return;
    }

    if (World->WorldType != EWorldType::PIE && World->WorldType != EWorldType::Game)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[DRIVE] WorldCleanup: %s (Type=%d) SessionEnded=%d Cleanup=%d"),
        *World->GetName(), (int32)World->WorldType, bSessionEnded ? 1 : 0, bCleanupResources ? 1 : 0);

    DriveHost->Stop();
    DriveHost.Reset();
    DriveHost = MakeUnique<FDriveSimulationHost>();
}

void UDriveGameInstance::HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
    if (!World)
    {
        return;
    }

    // Filtra: solo worlds de juego (PIE o Game)
    if (World->WorldType != EWorldType::PIE && World->WorldType != EWorldType::Game)
    {
        return;
    }

    #if WITH_EDITOR
        // En PIE desactivamos el auto-boot para poder controlar desde Blueprint
        if (World->WorldType == EWorldType::PIE)
        {
            DRIVE_LOG_UE(Log, TEXT("[DRIVE] PostWorldInit (PIE): auto-start disabled (manual control via Subsystem)."));
            return;
        }
    #endif

    DRIVE_LOG_UE(Log, TEXT("[DRIVE] PostWorldInit: %s (Type=%d IsGame=%d)"),
        *World->GetName(),
        (int32)World->WorldType,
        World->IsGameWorld() ? 1 : 0
    );

    if (DriveHost && !DriveHost->IsInitialized())
    {
        DriveHost->Initialize(World);
        DriveHost->Start();
    }
}

void UDriveGameInstance::Shutdown()
{
    if (PostWorldInitHandle.IsValid())
    {
        FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitHandle);
        PostWorldInitHandle.Reset();
    }

    if (DriveHost)
    {
        DriveHost->Stop();
        DriveHost.Reset();
    }

    if (WorldCleanupHandle.IsValid())
    {
        FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupHandle);
        WorldCleanupHandle.Reset();
    }

    Super::Shutdown();
}