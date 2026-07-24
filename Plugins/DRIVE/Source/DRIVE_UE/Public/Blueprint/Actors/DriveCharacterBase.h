#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/Interfaces/DriveControllable.h"
#include "DriveCharacterBase.generated.h"

class UDriveEntityComponent;

UCLASS(Blueprintable)
class DRIVE_UE_API ADriveCharacterBase : public ACharacter, public IDriveControllable
{
    GENERATED_BODY()

public:
    ADriveCharacterBase(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Base")
    UDriveEntityComponent* GetDriveEntityComponent() const { return DriveEntity; }

    UFUNCTION(BlueprintCallable, Category="DRIVE|Base")
    bool TryLinkToDrive() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|Components")
    TObjectPtr<UDriveEntityComponent> DriveEntity;

    virtual AActor* GetControllableActor_Implementation() const override { return const_cast<ADriveCharacterBase*>(this); }
    virtual bool CanBeControlled_Implementation() const override { return true; }
};