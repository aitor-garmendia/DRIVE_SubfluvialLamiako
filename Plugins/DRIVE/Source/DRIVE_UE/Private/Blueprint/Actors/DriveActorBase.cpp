#include "Blueprint/Actors/DriveActorBase.h"
#include "Blueprint/Components/DriveEntityComponent.h"

ADriveActorBase::ADriveActorBase()
{
    PrimaryActorTick.bCanEverTick = false;

    DriveEntity = CreateDefaultSubobject<UDriveEntityComponent>(TEXT("DriveEntity"));
}

bool ADriveActorBase::TryLinkToDrive() const
{
    return DriveEntity ? DriveEntity->TryLink() : false;
}