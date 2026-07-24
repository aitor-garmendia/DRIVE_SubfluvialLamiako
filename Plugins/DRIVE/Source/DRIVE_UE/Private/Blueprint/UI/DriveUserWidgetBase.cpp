#include "Blueprint/UI/DriveUserWidgetBase.h"
#include "UWorldSimulationSubsystem.h"
#include "Engine/World.h"

UWorldSimulationSubsystem* UDriveUserWidgetBase::GetWorldSimulationSubsystem() const
{
    UWorld* World = GetWorld();
    return World ? World->GetSubsystem<UWorldSimulationSubsystem>() : nullptr;
}

bool UDriveUserWidgetBase::HasWorldSimulationSubsystem() const
{
    return GetWorldSimulationSubsystem() != nullptr;
}

void UDriveUserWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();

    if (UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem())
    {
        BP_OnSubsystemAvailable(Subsystem);
    }
}