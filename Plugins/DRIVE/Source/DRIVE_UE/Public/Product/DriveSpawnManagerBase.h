#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DriveSpawnManagerBase.generated.h"

class APawn;

UENUM(BlueprintType)
enum class EDriveAgentKind : uint8
{
    Pedestrian UMETA(DisplayName="Pedestrian"),
    Vehicle    UMETA(DisplayName="Vehicle"),
    Drone      UMETA(DisplayName="Drone"),
    Observer   UMETA(DisplayName="Observer"),
};

USTRUCT(BlueprintType)
struct FDriveSpawnRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Spawn")
    EDriveAgentKind Kind = EDriveAgentKind::Pedestrian;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Spawn")
    TSubclassOf<APawn> PawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Spawn")
    FName SpawnTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Spawn")
    bool bFailIfNoSpawnPoint = true;
};

UCLASS(BlueprintType, Blueprintable)
class DRIVE_UE_API ADriveSpawnManagerBase : public AActor
{
    GENERATED_BODY()

public:
    ADriveSpawnManagerBase();

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn")
    TArray<AActor*> SpawnPointArray;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn")
    bool bAutoRegisterSpawnPointsByTag = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn", meta=(EditCondition="bAutoRegisterSpawnPointsByTag"))
    FName AutoRegisterTag = TEXT("DRIVE_Spawn");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn|Classes")
    TSubclassOf<APawn> PedestrianPawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn|Classes")
    TSubclassOf<APawn> VehiclePawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn|Classes")
    TSubclassOf<APawn> DronePawnClass;

    // API BP
    UFUNCTION(BlueprintCallable, Category="DRIVE|Spawn")
    void RefreshSpawnPoints();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Spawn")
    bool TryPickSpawnTransform(FName SpawnTag, FTransform& OutTransform) const;

    UFUNCTION(BlueprintCallable, Category="DRIVE|Spawn")
    APawn* SpawnPawnAt(const FDriveSpawnRequest& Request, const FTransform& SpawnTransform);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Spawn")
    APawn* SpawnAgent(const FDriveSpawnRequest& Request);

    UFUNCTION(BlueprintImplementableEvent, Category="DRIVE|Spawn")
    void OnAgentSpawned(APawn* SpawnedPawn, const FDriveSpawnRequest& Request);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Spawn")
    void SortSpawnPoints(UPARAM(ref) TArray<ADriveSpawnPoint*>& spawnPoints);

protected:
    virtual void BeginPlay() override;
};