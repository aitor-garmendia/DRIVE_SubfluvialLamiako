#include "Actors/DriveSpawnPoint.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"

ADriveSpawnPoint::ADriveSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;

    //Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    //SetRootComponent(Root);

    //Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
    //Arrow->SetupAttachment(Root);
}