#pragma once

#include "CoreMinimal.h"
#include "Blueprint/Actors/DrivePawnBase.h"
#include "Engine/EngineTypes.h" 
#include "DriveFreeCamera.generated.h"

UCLASS(BlueprintType, Blueprintable)
class DRIVE_UE_API ADriveFreeCamera : public ADrivePawnBase
{
    GENERATED_BODY()

public:
    ADriveFreeCamera();

    UFUNCTION(BlueprintCallable, Category="DRIVE|FreeCamera")
    void ChangeViewMode(EViewModeIndex ViewMode) const;
    UFUNCTION(BlueprintCallable, Category="DRIVE|FreeCamera")
    int GetCurrentViewMode() const;

private:
    UGameViewportClient* GetGameViewport() const;
};