#include "Blueprint/Game/DriveGameModeBase.h"

ADriveGameModeBase::ADriveGameModeBase()
{
}

void ADriveGameModeBase::BeginPlay()
{
    Super::BeginPlay();
    BP_OnDriveGameModeBeginPlay();
}