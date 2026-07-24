#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DriveGameModeBase.generated.h"

UCLASS(Blueprintable)
class DRIVE_UE_API ADriveGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ADriveGameModeBase();
    UFUNCTION(BlueprintImplementableEvent, Category="DRIVE|Game")
    void BP_OnDriveGameModeBeginPlay();

protected:
    virtual void BeginPlay() override;
};