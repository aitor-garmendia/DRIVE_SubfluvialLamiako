#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/Interfaces/DriveControllable.h"
#include "DrivePawnBase.generated.h"

class UDriveEntityComponent;

UCLASS(Blueprintable)
class DRIVE_UE_API ADrivePawnBase : public APawn, public IDriveControllable
{
    GENERATED_BODY()

public:
    ADrivePawnBase();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Base")
    UDriveEntityComponent* GetDriveEntityComponent() const { return DriveEntity; }

    UFUNCTION(BlueprintCallable, Category="DRIVE|Base")
    bool TryLinkToDrive() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|Components")
    TObjectPtr<UDriveEntityComponent> DriveEntity;

    virtual AActor* GetControllableActor_Implementation() const override { return const_cast<ADrivePawnBase*>(this); }
    virtual bool CanBeControlled_Implementation() const override { return true; }
};