#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DriveAgentSwitchManagerBase.generated.h"

class APlayerController;
class APawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDriveOnActivePawnChanged, APawn*, OldPawn, APawn*, NewPawn);

UCLASS(BlueprintType, Blueprintable)
class DRIVE_UE_API ADriveAgentSwitchManagerBase : public AActor
{
    GENERATED_BODY()

public:
    ADriveAgentSwitchManagerBase();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Switch")
    float CameraBlendTime = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Switch")
    bool bAutoFindPlayerController = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Switch", meta=(EditCondition="!bAutoFindPlayerController"))
    TSoftObjectPtr<APlayerController> ExplicitPlayerController;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Switch")
    FDriveOnActivePawnChanged OnActivePawnChanged;

    UFUNCTION(BlueprintCallable, Category="DRIVE|Switch")
    bool SwitchToPawn(APawn* NewPawn);

    UFUNCTION(BlueprintPure, Category="DRIVE|Switch")
    APawn* GetActivePawn() const { return ActivePawn.Get(); }

protected:
    virtual void BeginPlay() override;

    APlayerController* ResolvePlayerController() const;

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<APawn> ActivePawn;
};