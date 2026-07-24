#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DriveControllable.generated.h"

UINTERFACE(BlueprintType)
class DRIVE_UE_API UDriveControllable : public UInterface
{
    GENERATED_BODY()
};

class DRIVE_UE_API IDriveControllable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="DRIVE|Control")
    AActor* GetControllableActor() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="DRIVE|Control")
    bool CanBeControlled() const;
};