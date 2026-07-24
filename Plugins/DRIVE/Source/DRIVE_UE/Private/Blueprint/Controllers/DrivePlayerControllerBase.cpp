#include "Blueprint/Controllers/DrivePlayerControllerBase.h"
#include "GameFramework/Pawn.h"

bool ADrivePlayerControllerBase::PossessActorSafely(AActor* NewActor)
{
    if (!NewActor) return false;

    APawn* NewPawn = Cast<APawn>(NewActor);
    if (!NewPawn) return false;

    AActor* Old = GetPawn();
    BP_OnBeforePossess(NewActor);

    if (Old)
    {
        BP_OnBeforeUnpossess(Old);
        UnPossess();
        BP_OnAfterUnpossess(Old);
    }

    Possess(NewPawn);
    BP_OnAfterPossess(NewActor);
    return true;
}

void ADrivePlayerControllerBase::UnpossessSafely()
{
    AActor* Old = GetPawn();
    if (!Old) return;

    BP_OnBeforeUnpossess(Old);
    UnPossess();
    BP_OnAfterUnpossess(Old);
}

AActor* ADrivePlayerControllerBase::GetCurrentPossessedActor() const
{
    return GetPawn();
}