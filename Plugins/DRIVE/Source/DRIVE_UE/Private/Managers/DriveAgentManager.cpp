#include "Managers/DriveAgentManager.h"
#include "UWorldSimulationSubsystem.h"
#include "Actors/DriveSpawnPoint.h"
#include "EngineUtils.h"
#include "Engine/World.h"

void UDriveAgentManager::Initialize(UWorldSimulationSubsystem* InSubsystem)
{
    Subsystem = InSubsystem;
    Agents.Reset();
    VisualActorsByAgentId.Reset();
    ActiveAgent = FDriveAgentHandle{};
    NextAgentId = 1;
}

ADriveSpawnPoint* UDriveAgentManager::FindSpawnPoint(EDriveAgentType Type, FName SpawnTag) const
{
    if (!Subsystem) return nullptr;
    UWorld* World = Subsystem->GetWorld();
    if (!World) return nullptr;

    for (TActorIterator<ADriveSpawnPoint> It(World); It; ++It)
    {
        ADriveSpawnPoint* SP = *It;
        if (!SP) continue;

        if (SP->AgentType == Type)
        {
            if (SpawnTag.IsNone() || SP->SpawnTag == SpawnTag)
            {
                return SP;
            }
        }
    }
    return nullptr;
}

FDriveAgentHandle UDriveAgentManager::SpawnAgent(EDriveAgentType Type, FName SpawnTag)
{
    FDriveAgentHandle Handle;
    if (!Subsystem) return Handle;

    UWorld* World = Subsystem->GetWorld();
    if (!World) return Handle;

    ADriveSpawnPoint* SP = FindSpawnPoint(Type, SpawnTag);
    if (!SP) return Handle;

    Handle.Id = NextAgentId++;
    Agents.Add(Handle);

    return Handle;
}

bool UDriveAgentManager::DespawnAgent(FDriveAgentHandle Handle)
{
    if (!Handle.IsValid()) return false;

    DetachVisualization(Handle);

    Agents.RemoveAll([&](const FDriveAgentHandle& H) { return H.Id == Handle.Id; });

    if (ActiveAgent.Id == Handle.Id)
    {
        ActiveAgent = FDriveAgentHandle{};
    }

    return true;
}

bool UDriveAgentManager::AttachVisualization(FDriveAgentHandle Handle, TSubclassOf<AActor> VisualizationClass)
{
    if (!Subsystem || !Handle.IsValid() || !*VisualizationClass) return false;

    UWorld* World = Subsystem->GetWorld();
    if (!World) return false;

    if (VisualActorsByAgentId.Contains(Handle.Id) && VisualActorsByAgentId[Handle.Id].IsValid())
        return true;

    FTransform SpawnXform = FTransform::Identity;
    AActor* Actor = World->SpawnActor<AActor>(VisualizationClass, SpawnXform);
    if (!Actor) return false;

    VisualActorsByAgentId.Add(Handle.Id, Actor);
    return true;
}

bool UDriveAgentManager::DetachVisualization(FDriveAgentHandle Handle)
{
    if (!Handle.IsValid()) return false;

    if (TWeakObjectPtr<AActor>* Ptr = VisualActorsByAgentId.Find(Handle.Id))
    {
        if (Ptr->IsValid())
        {
            Ptr->Get()->Destroy();
        }
        VisualActorsByAgentId.Remove(Handle.Id);
        return true;
    }
    return false;
}

TArray<FDriveAgentHandle> UDriveAgentManager::GetAllAgents() const
{
    return Agents;
}

bool UDriveAgentManager::SetActiveAgent(FDriveAgentHandle Handle)
{
    if (!Handle.IsValid()) return false;

    const bool bExists = Agents.ContainsByPredicate([&](const FDriveAgentHandle& H) { return H.Id == Handle.Id; });
    if (!bExists) return false;

    ActiveAgent = Handle;
    return true;
}