#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/DriveSessionTypes.h"
#include "DriveAgentPreset.generated.h"

UCLASS(BlueprintType)
class DRIVE_UE_API UDriveAgentPreset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Preset|Agent")
    EDriveAgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Preset|Agent")
    bool bCanBePossessed = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Preset|Agent")
    bool bAutoLinkOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Preset|Agent")
    bool bAutoRegisterOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Preset|Sync")
    bool bAutoSyncTransform = true;
};