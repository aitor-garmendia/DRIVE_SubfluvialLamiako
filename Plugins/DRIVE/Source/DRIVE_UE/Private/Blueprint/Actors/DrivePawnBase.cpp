#include "Blueprint/Actors/DrivePawnBase.h"
#include "Blueprint/Components/DriveEntityComponent.h"

ADrivePawnBase::ADrivePawnBase()
{
    PrimaryActorTick.bCanEverTick = false;
    DriveEntity = CreateDefaultSubobject<UDriveEntityComponent>(TEXT("DriveEntity"));
}

bool ADrivePawnBase::TryLinkToDrive() const
{
    return DriveEntity ? DriveEntity->TryLink() : false;
}