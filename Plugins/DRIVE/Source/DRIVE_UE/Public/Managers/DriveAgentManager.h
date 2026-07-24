#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "Types/DriveSessionTypes.h"

#include "DriveAgentManager.generated.h"

class UWorldSimulationSubsystem;
class ADriveSpawnPoint;
class AActor;

UCLASS(BlueprintType)
class DRIVE_UE_API UDriveAgentManager : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UWorldSimulationSubsystem* InSubsystem);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Agents")
    FDriveAgentHandle SpawnAgent(EDriveAgentType Type, FName SpawnTag);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Agents")
    bool DespawnAgent(FDriveAgentHandle Handle);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Agents")
    bool AttachVisualization(FDriveAgentHandle Handle, TSubclassOf<AActor> VisualizationClass);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Agents")
    bool DetachVisualization(FDriveAgentHandle Handle);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Agents")
    TArray<FDriveAgentHandle> GetAllAgents() const;

    UFUNCTION(BlueprintCallable, Category="DRIVE|Agents")
    bool SetActiveAgent(FDriveAgentHandle Handle);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Agents")
    FDriveAgentHandle GetActiveAgent() const { return ActiveAgent; }

private:
    UPROPERTY()
    UWorldSimulationSubsystem* Subsystem = nullptr;

    UPROPERTY()
    TArray<FDriveAgentHandle> Agents;

    UPROPERTY()
    TMap<int32, TWeakObjectPtr<AActor>> VisualActorsByAgentId;

    UPROPERTY()
    FDriveAgentHandle ActiveAgent;

    int32 NextAgentId = 1;

    ADriveSpawnPoint* FindSpawnPoint(EDriveAgentType Type, FName SpawnTag) const;
};