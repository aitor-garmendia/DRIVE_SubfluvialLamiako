#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FDriveSimulationHost.h"
#include "DriveGameInstance.generated.h"

UCLASS()
class DRIVE_TEMPLATERUNTIME_API UDriveGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

private:
    void HandlePostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
    void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

    FDelegateHandle WorldCleanupHandle;
    FDelegateHandle PostWorldInitHandle;            
    TUniquePtr<FDriveSimulationHost> DriveHost;
};