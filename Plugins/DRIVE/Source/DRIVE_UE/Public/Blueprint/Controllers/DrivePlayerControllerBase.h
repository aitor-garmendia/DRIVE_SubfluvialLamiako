#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DrivePlayerControllerBase.generated.h"

UCLASS(Blueprintable)
class DRIVE_UE_API ADrivePlayerControllerBase : public APlayerController
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="DRIVE|Control")
    bool PossessActorSafely(AActor* NewActor);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Control")
    void UnpossessSafely();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Control")
    AActor* GetCurrentPossessedActor() const;

    UFUNCTION(BlueprintImplementableEvent, Category="DRIVE|Control")
    void BP_OnBeforePossess(AActor* NewActor);

    UFUNCTION(BlueprintImplementableEvent, Category="DRIVE|Control")
    void BP_OnAfterPossess(AActor* NewActor);

    UFUNCTION(BlueprintImplementableEvent, Category="DRIVE|Control")
    void BP_OnBeforeUnpossess(AActor* OldActor);

    UFUNCTION(BlueprintImplementableEvent, Category="DRIVE|Control")
    void BP_OnAfterUnpossess(AActor* OldActor);
};