#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/DriveSessionTypes.h"
#include "DriveSpawnPoint.generated.h"

class USceneComponent;
class UArrowComponent;

UCLASS(BlueprintType, Blueprintable)
class DRIVE_UE_API ADriveSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    ADriveSpawnPoint();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn")
    EDriveAgentType AgentType = EDriveAgentType::Vehicle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn")
    bool bAllowAnyType = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn")
    FName SpawnTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn")
    int PositionInList = 0;

    UFUNCTION(BlueprintCallable, Category="DRIVE|Spawn")
    FTransform GetSpawnTransform() const { return GetActorTransform(); }

protected:
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn")
    //TObjectPtr<USceneComponent> Root;

    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|Spawn")
    //TObjectPtr<UArrowComponent> Arrow;
};