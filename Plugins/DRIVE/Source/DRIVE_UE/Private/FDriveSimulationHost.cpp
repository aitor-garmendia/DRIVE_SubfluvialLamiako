#include "FDriveSimulationHost.h"
#include "Engine/World.h"
#include "UWorldSimulationSubsystem.h"
#include "Tickable.h"
#include "Containers/Ticker.h"

#include "DriveGlobalSettings.h"
#include "DriveGlobalConfigAsset.h"
#include "Systems/FDriveDummySystem.h"
#include "Systems/DriveSystemContext.h"
#include "Systems/FDriveMovementSystem.h"
#include "Scenario/DriveScenarioDefinition.h"
#include "Logging/DriveLog.h"

#include "Systems/FDriveMessageBusProbeSystem.h"

FDriveSimulationHost::FDriveSimulationHost(): World(nullptr), WorldSubsystem(nullptr), GlobalConfig(nullptr), bInitialized(false)
{
}

FDriveSimulationHost::~FDriveSimulationHost()
{
    UnregisterTick();
    if (RetryTickHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(RetryTickHandle);
        RetryTickHandle.Reset();
    }
}

void FDriveSimulationHost::Initialize(UWorld* InWorld)
{
    if (bInitialized || !InWorld)
    {
        return;
    }

    World = InWorld;

    LoadGlobalConfig();
    BindSubsystems();

    if (!WorldSubsystem)
    {
        TryBindLater();
        return;
    }

    InitializeSimulation();
    bInitialized = true;
}

void FDriveSimulationHost::LoadGlobalConfig()
{
    GlobalConfig = nullptr;

    const UDriveGlobalSettings* Settings = GetDefault<UDriveGlobalSettings>();
    if (Settings && Settings->GlobalConfigAsset.IsValid())
    {
        GlobalConfig = Settings->GlobalConfigAsset.Get();
    }
    else if (Settings && !Settings->GlobalConfigAsset.IsNull())
    {
        GlobalConfig = Settings->GlobalConfigAsset.LoadSynchronous();
    }

    if (!GlobalConfig)
    {
        DRIVE_LOG_HOST(Warning, TEXT("[DRIVE] GlobalConfigAsset not set. Using defaults inside asset class (fallback)."));
        return;
    }

    FString Error;
    if (!GlobalConfig->IsValidConfig(Error))
    {
        DRIVE_LOG_HOST(Error, TEXT("[DRIVE] GlobalConfig invalid: %s. Falling back to defaults."), *Error);
        GlobalConfig = nullptr;
        return;
    }
    DRIVE_LOG_HOST(Log, TEXT("[DRIVE] GlobalConfig loaded: FixedDelta=%.6f TimeScale=%.2f MaxSteps=%d MaxAgents=%d"), GlobalConfig->FixedDeltaTime, GlobalConfig->DefaultTimeScale, GlobalConfig->MaxFixedStepsPerFrame, GlobalConfig->MaxAgents);
}

void FDriveSimulationHost::TryBindLater()
{
    if (RetryTickHandle.IsValid())
    {
        return;
    }

    RetryTickHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([this](float)
        {
            if (!World) return true;

            BindSubsystems();

            if (WorldSubsystem)
            {
                InitializeSimulation();
                bInitialized = true;
                DRIVE_LOG_HOST(Warning, TEXT("[DRIVE] Host bound after retry"));

                FTSTicker::GetCoreTicker().RemoveTicker(RetryTickHandle);
                RetryTickHandle.Reset();

                return false;
            }
            DRIVE_LOG_HOST(Log, TEXT("[DRIVE] Host retry: subsystem still NULL..."));
            return true;
        })
    );
}


void FDriveSimulationHost::BindSubsystems()
{
    DRIVE_LOG_HOST(Log, TEXT("[DRIVE] BindSubsystems World=%s Type=%d IsGame=%d"), *World->GetName(), (int32)World->WorldType, World->IsGameWorld() ? 1 : 0);
    WorldSubsystem = World->GetSubsystem<UWorldSimulationSubsystem>();

    if (!WorldSubsystem)
    {
        DRIVE_LOG_HOST(Error, TEXT("[DRIVE] WorldSubsystem is NULL!"));
    }
    else
    {
        DRIVE_LOG_HOST(Log, TEXT("[DRIVE] WorldSubsystem OK"));
    }
}

void FDriveSimulationHost::InitializeSimulation()
{
    if (WorldSubsystem)
    {
        WorldSubsystem->InitializeSimulation(GlobalConfig);
        BuildAndStartSystems();
    }
}

void FDriveSimulationHost::Start()
{
    if (!WorldSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DRIVE][Host] Start() ignored: WorldSubsystem null"));
        return;
    }

    if (WorldSubsystem->IsRunning() && TickHandle.IsValid())
    {
        UE_LOG(LogTemp, Verbose, TEXT("[DRIVE][Host] Start() ignored: already running + tick registered"));
        return;
    }

    WorldSubsystem->StartSimulation();
    RegisterTick();
}

void FDriveSimulationHost::RegisterTick()
{
    if (TickHandle.IsValid())
        return;

    TickHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([this](float DeltaSeconds)
        {
            if (!WorldSubsystem || !WorldSubsystem->IsRunning())
                return true;

            WorldSubsystem->Advance(DeltaSeconds);
            return true;
        })
    );
    DRIVE_LOG_HOST(Log, TEXT("[DRIVE] Host Tick registered"));
}

void FDriveSimulationHost::UnregisterTick()
{
    if (TickHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
        TickHandle.Reset();
        DRIVE_LOG_HOST(Log, TEXT("[DRIVE] Host Tick unregistered"));
    }
}

void FDriveSimulationHost::Pause()
{
    if (WorldSubsystem)
    {
        WorldSubsystem->PauseSimulation();
    }
}

void FDriveSimulationHost::Stop()
{
    if (WorldSubsystem)
    {
        ShutdownAndUnregisterSystems();
        WorldSubsystem->StopSimulation();
    }
    UnregisterTick();
}

void FDriveSimulationHost::BuildAndStartSystems()
{
    if (!WorldSubsystem || bSystemsReady)
    {
        return;
    }

    BuildActiveSystems();
    WorldSubsystem->UnregisterAllSystems();
    for (const TSharedRef<FDriveSystemBase>& Sys : ActiveSystems)
    {
        DRIVE_LOG_HOST(Log, TEXT("[DRIVE][Host] RegisterSystem: %s"), *Sys->GetSystemId().ToString());
        WorldSubsystem->RegisterSystem(Sys);
    }

    bSystemsReady = InitSystemsExecutionPlan();
    if (bSystemsReady)
    {
        //WorldSubsystem->StartSimulation();
        DRIVE_LOG_HOST(Log, TEXT("[DRIVE][Host] StartSimulation called (systems ready)"));
    }
}

void FDriveSimulationHost::BuildActiveSystems()
{
    ActiveSystems.Reset();
    ActiveSystems.Add(MakeShared<FDriveDummySystem>());
    ActiveSystems.Add(MakeShared<FDriveMovementSystem>());
    
   //ActiveSystems.Add(MakeShared<FDriveMessageBusProbeSystem>()); 

    ActiveSystems.Sort([](const TSharedRef<FDriveSystemBase>& A, const TSharedRef<FDriveSystemBase>& B)
    {
        return A->GetInitOrder() < B->GetInitOrder();
    });
   
    DRIVE_LOG_HOST(Log, TEXT("[DRIVE][Host] ActiveSystems=%d"), ActiveSystems.Num());
}

bool FDriveSimulationHost::InitSystemsExecutionPlan()
{
    if (!WorldSubsystem)
    {
        return false;
    }

    FDriveSystemContext Ctx;
    Ctx.GlobalConfig = GlobalConfig;
    Ctx.Clock = &WorldSubsystem->GetClock();
    Ctx.Bus = WorldSubsystem->GetMessageBus();
    Ctx.Runtime = &WorldSubsystem->GetRuntime();
    Ctx.SimState = &WorldSubsystem->GetRuntime().GetSimState();
    Ctx.EcsWorld = WorldSubsystem->GetRuntime().GetEcsWorld();

    const bool bOk = WorldSubsystem->GetRuntime().BuildExecutionPlanAndInitializeSystems(Ctx);

    DRIVE_LOG_HOST(Log, TEXT("[DRIVE][Host] BuildExecutionPlanAndInitializeSystems -> %s"), bOk ? TEXT("OK") : TEXT("FAIL"));
    return bOk;
}

void FDriveSimulationHost::ShutdownAndUnregisterSystems()
{
    if (!WorldSubsystem || !bSystemsReady)
    {
        return;
    }

    for (int32 i = ActiveSystems.Num() - 1; i >= 0; --i)
    {
        if (ActiveSystems[i]->IsInitialized())
        {
            DRIVE_LOG_HOST(Log, TEXT("[DRIVE][Host] Shutdown: %s"), *ActiveSystems[i]->GetSystemId().ToString());
            ActiveSystems[i]->Shutdown();
        }
    }

    WorldSubsystem->UnregisterAllSystems();
    ActiveSystems.Reset();
    bSystemsReady = false;

    DRIVE_LOG_HOST(Log, TEXT("[DRIVE][Host] Systems shutdown + unregistered"));
}