#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Systems/DriveSystemBase.h"

class UWorld;
class UWorldSimulationSubsystem;
class UDriveGlobalConfigAsset;
class FDriveSystemBase;

class DRIVE_UE_API FDriveSimulationHost
{
public:
    FDriveSimulationHost();
    ~FDriveSimulationHost();

    void Initialize(UWorld* InWorld);
    void Start();
    void Pause();
    void Stop();

    bool IsInitialized() const { return bInitialized; }

    const UDriveGlobalConfigAsset* GetGlobalConfig() const { return GlobalConfig; }

private:
    void BindSubsystems();
    void InitializeSimulation();
    void TryBindLater();

    void LoadGlobalConfig();

    void BuildAndStartSystems();
    void ShutdownAndUnregisterSystems();

    void BuildActiveSystems(); 
    bool InitSystemsExecutionPlan();

private:
    UWorld* World = nullptr;
    UWorldSimulationSubsystem* WorldSubsystem = nullptr;
    
    UDriveGlobalConfigAsset* GlobalConfig = nullptr;
    bool bInitialized = false;

	FTSTicker::FDelegateHandle TickHandle;
	void RegisterTick();
	void UnregisterTick();

    TArray<TSharedRef<FDriveSystemBase>> ActiveSystems;
    bool bSystemsReady = false;
    FTSTicker::FDelegateHandle RetryTickHandle;
};