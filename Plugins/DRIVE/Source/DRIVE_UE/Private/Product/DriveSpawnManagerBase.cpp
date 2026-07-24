#include "Product/DriveSpawnManagerBase.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/DriveLog.h"
#include "Actors/DriveSpawnPoint.h"

ADriveSpawnManagerBase::ADriveSpawnManagerBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ADriveSpawnManagerBase::BeginPlay()
{
    Super::BeginPlay();
    RefreshSpawnPoints();
}

void ADriveSpawnManagerBase::RefreshSpawnPoints()
{
    SpawnPointArray.Reset();

    if (!GetWorld())
    {
        return;
    }

    if (bAutoRegisterSpawnPointsByTag)
    {
        for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        {
            AActor* A = *It;
            if (!A) continue;

            if (AutoRegisterTag != NAME_None && A->ActorHasTag(AutoRegisterTag))
            {
                SpawnPointArray.Add(A);
            }
        }
    }
}

bool ADriveSpawnManagerBase::TryPickSpawnTransform(FName SpawnTag, FTransform& OutTransform) const
{
    TArray<AActor*> Candidates;
    Candidates.Reserve(SpawnPointArray.Num());

    for (AActor* P : SpawnPointArray)
    {
        if (!P) continue;

        if (SpawnTag == NAME_None || P->ActorHasTag(SpawnTag))
        {
            Candidates.Add(P);
        }
    }

    if (Candidates.Num() == 0)
    {
        return false;
    }

    const int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
    OutTransform = Candidates[Index]->GetActorTransform();
    return true;
}

APawn* ADriveSpawnManagerBase::SpawnPawnAt(const FDriveSpawnRequest& Request, const FTransform& SpawnTransform)
{
    if (!GetWorld() || !Request.PawnClass)
    {
        return nullptr;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    APawn* Pawn = GetWorld()->SpawnActor<APawn>(Request.PawnClass, SpawnTransform, Params);
    if (Pawn)
    {
        OnAgentSpawned(Pawn, Request);
    }
    return Pawn;
}

APawn* ADriveSpawnManagerBase::SpawnAgent(const FDriveSpawnRequest& Request)
{
    FTransform T;
    const bool bFound = TryPickSpawnTransform(Request.SpawnTag, T);

    if (!bFound)
    {
        if (Request.bFailIfNoSpawnPoint)
        {
            DRIVE_LOG_MANAGER(Log, TEXT("[DRIVE][SpawnManager] No spawn point found (Tag=%s)"), *Request.SpawnTag.ToString());
        }
        return nullptr;
    }

    return SpawnPawnAt(Request, T);
}

void ADriveSpawnManagerBase::SortSpawnPoints(TArray<ADriveSpawnPoint*>& spawnPoints)
{
    spawnPoints.Sort([](const ADriveSpawnPoint& A, const ADriveSpawnPoint& B)
    {
        return A.PositionInList < B.PositionInList;
    });
}