#include "Product/DriveSessionManagerBase.h"

#include "UWorldSimulationSubsystem.h"
#include "DriveGlobalConfigAsset.h"
#include "Scenario/DriveScenarioAsset.h"

#include "Engine/World.h"
#include "Logging/DriveLog.h"

ADriveSessionManagerBase::ADriveSessionManagerBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ADriveSessionManagerBase::BeginPlay()
{
    Super::BeginPlay();

    if (!EnsureSubsystem())
    {
        return;
    }

    BindSubsystemEvents();

    if (bAutoInitialize)
    {
        InitializeSession();
    }

    if (bAutoLoadScenario)
    {
        LoadScenario();
    }

    if (bAutoStart)
    {
        StartSession();
    }
}

void ADriveSessionManagerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnbindSubsystemEvents();
    Super::EndPlay(EndPlayReason);
}

bool ADriveSessionManagerBase::EnsureSubsystem()
{
    if (CachedSubsystem)
    {
        return true;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        DRIVE_LOG_MANAGER(Error, TEXT("[DRIVE][SessionManager] No valid World."));
        return false;
    }

    CachedSubsystem = World->GetSubsystem<UWorldSimulationSubsystem>();
    if (!CachedSubsystem)
    {
        DRIVE_LOG_MANAGER(Error, TEXT("[DRIVE][SessionManager] UWorldSimulationSubsystem not found."));
        return false;
    }

    return true;
}


// API
bool ADriveSessionManagerBase::InitializeSession()
{
    if (!EnsureSubsystem())
        return false;

    CachedSubsystem->BP_InitializeSimulation(GlobalConfig);
    return true;
}

bool ADriveSessionManagerBase::LoadScenario()
{
    if (!EnsureSubsystem() || !ScenarioAsset)
        return false;

    return CachedSubsystem->BP_TryLoadScenario(ScenarioAsset);
}

bool ADriveSessionManagerBase::StartSession()
{
    if (!EnsureSubsystem())
        return false;

    return CachedSubsystem->BP_TryStartSimulation();
}

void ADriveSessionManagerBase::PauseSession()
{
    if (!EnsureSubsystem())
        return;

    CachedSubsystem->BP_PauseSimulation();
}

void ADriveSessionManagerBase::ResumeSession()
{
    if (!EnsureSubsystem())
        return;

    CachedSubsystem->BP_ResumeSimulation();
}

void ADriveSessionManagerBase::StopSession()
{
    if (!EnsureSubsystem())
        return;

    CachedSubsystem->BP_StopSimulation();
}

int32 ADriveSessionManagerBase::StepManual(int32 NumSteps)
{
    if (!EnsureSubsystem())
        return 0;

    return CachedSubsystem->BP_StepOnce(NumSteps);
}

bool ADriveSessionManagerBase::ResetSession(bool bReloadScenario)
{
    if (!EnsureSubsystem())
        return false;

    return CachedSubsystem->BP_ResetSimulation(bReloadScenario);
}


// EVENT BINDING
void ADriveSessionManagerBase::BindSubsystemEvents()
{
    if (!CachedSubsystem)
        return;

    CachedSubsystem->OnSimulationStarted.AddDynamic(this, &ADriveSessionManagerBase::HandleSimStarted);
    CachedSubsystem->OnSimulationPaused.AddDynamic(this, &ADriveSessionManagerBase::HandleSimPaused);
    CachedSubsystem->OnSimulationResumed.AddDynamic(this, &ADriveSessionManagerBase::HandleSimResumed);
    CachedSubsystem->OnSimulationStopped.AddDynamic(this, &ADriveSessionManagerBase::HandleSimStopped);
    CachedSubsystem->OnScenarioLoaded.AddDynamic(this, &ADriveSessionManagerBase::HandleScenarioLoaded);
}

void ADriveSessionManagerBase::UnbindSubsystemEvents()
{
    if (!CachedSubsystem)
        return;

    CachedSubsystem->OnSimulationStarted.RemoveDynamic(this, &ADriveSessionManagerBase::HandleSimStarted);
    CachedSubsystem->OnSimulationPaused.RemoveDynamic(this, &ADriveSessionManagerBase::HandleSimPaused);
    CachedSubsystem->OnSimulationResumed.RemoveDynamic(this, &ADriveSessionManagerBase::HandleSimResumed);
    CachedSubsystem->OnSimulationStopped.RemoveDynamic(this, &ADriveSessionManagerBase::HandleSimStopped);
    CachedSubsystem->OnScenarioLoaded.RemoveDynamic(this, &ADriveSessionManagerBase::HandleScenarioLoaded);
}


// EVENT HANDLERS
void ADriveSessionManagerBase::HandleSimStarted()
{
    OnSessionStarted.Broadcast();
}

void ADriveSessionManagerBase::HandleSimPaused()
{
    OnSessionPaused.Broadcast();
}

void ADriveSessionManagerBase::HandleSimResumed()
{
    OnSessionResumed.Broadcast();
}

void ADriveSessionManagerBase::HandleSimStopped()
{
    OnSessionStopped.Broadcast();
}

void ADriveSessionManagerBase::HandleScenarioLoaded(FName ScenarioId)
{
    OnScenarioLoaded.Broadcast(ScenarioId);
}