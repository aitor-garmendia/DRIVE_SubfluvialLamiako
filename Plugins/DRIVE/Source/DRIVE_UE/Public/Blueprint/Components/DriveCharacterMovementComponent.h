#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DriveCharacterMovementComponent.generated.h"

UCLASS(ClassGroup=(Movement), meta=(BlueprintSpawnableComponent))
class DRIVE_UE_API UDriveCharacterMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UDriveCharacterMovementComponent();
};