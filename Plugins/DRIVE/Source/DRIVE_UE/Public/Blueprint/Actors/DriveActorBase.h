#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/Interfaces/DriveControllable.h"
#include "DriveActorBase.generated.h"

class UDriveEntityComponent;

UCLASS(Blueprintable)
class DRIVE_UE_API ADriveActorBase : public AActor, public IDriveControllable
{
    GENERATED_BODY()

public:
    ADriveActorBase();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Base")
    UDriveEntityComponent* GetDriveEntityComponent() const { return DriveEntity; }

    UFUNCTION(BlueprintCallable, Category="DRIVE|Base")
    bool TryLinkToDrive() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|Components")
    TObjectPtr<UDriveEntityComponent> DriveEntity;

    virtual AActor* GetControllableActor_Implementation() const override { return const_cast<ADriveActorBase*>(this); }
    virtual bool CanBeControlled_Implementation() const override { return true; }
};