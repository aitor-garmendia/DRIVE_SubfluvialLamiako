#include "Blueprint/Actors/DriveCharacterBase.h"
#include "Blueprint/Components/DriveEntityComponent.h"
#include "Blueprint/Components/DriveCharacterMovementComponent.h"

ADriveCharacterBase::ADriveCharacterBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UDriveCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = false;
    DriveEntity = CreateDefaultSubobject<UDriveEntityComponent>(TEXT("DriveEntity"));
}

bool ADriveCharacterBase::TryLinkToDrive() const
{
    return DriveEntity ? DriveEntity->TryLink() : false;
}