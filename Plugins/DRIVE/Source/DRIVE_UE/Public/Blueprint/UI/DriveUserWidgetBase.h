#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DriveUserWidgetBase.generated.h"

class UWorldSimulationSubsystem;

UCLASS(Blueprintable)
class DRIVE_UE_API UDriveUserWidgetBase : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="DRIVE|UI")
    UWorldSimulationSubsystem* GetWorldSimulationSubsystem() const;

    UFUNCTION(BlueprintCallable, Category="DRIVE|UI")
    bool HasWorldSimulationSubsystem() const;

    UFUNCTION(BlueprintImplementableEvent, Category="DRIVE|UI")
    void BP_OnSubsystemAvailable(UWorldSimulationSubsystem* Subsystem);

protected:
    virtual void NativeConstruct() override;
};