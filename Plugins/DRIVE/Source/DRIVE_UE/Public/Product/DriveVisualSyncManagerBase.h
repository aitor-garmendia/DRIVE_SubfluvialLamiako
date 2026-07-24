#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DriveVisualSyncManagerBase.generated.h"

class UDriveEntityComponent;

USTRUCT(BlueprintType)
struct FDriveVisualBinding
{
    GENERATED_BODY()

    // ID lógico de la entidad 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    int32 EntityId = -1;

    // Actor visual asociado
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    TWeakObjectPtr<AActor> Actor;

    // Componente DRIVE del actor 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    TWeakObjectPtr<UDriveEntityComponent> EntityComponent;

    // Último transform objetivo 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    FTransform TargetTransform = FTransform::Identity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    bool bHasTarget = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    float LastUpdateTime = 0.f;
};

UCLASS(BlueprintType, Blueprintable)
class DRIVE_UE_API ADriveVisualSyncManagerBase : public AActor
{
    GENERATED_BODY()

public:
    ADriveVisualSyncManagerBase();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    bool bTickEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    bool bEnableSmoothing = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync", meta=(EditCondition="bEnableSmoothing", ClampMin="0.0"))
    float LocationInterpSpeed = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync", meta=(EditCondition="bEnableSmoothing", ClampMin="0.0"))
    float RotationInterpSpeed = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync", meta=(ClampMin="0.0"))
    float SnapDistance = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    bool bDebug = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|VisualSync")
    TArray<FDriveVisualBinding> Bindings;

    // Registra un Actor 
    UFUNCTION(BlueprintCallable, Category="DRIVE|VisualSync")
    bool RegisterActor(AActor* InActor, int32 EntityId = -1);

    // Desregistra por EntityId
    UFUNCTION(BlueprintCallable, Category="DRIVE|VisualSync")
    bool UnregisterByEntityId(int32 EntityId);

    // Desregistra por Actor
    UFUNCTION(BlueprintCallable, Category="DRIVE|VisualSync")
    bool UnregisterActor(AActor* InActor);

    // Limpia todo
    UFUNCTION(BlueprintCallable, Category="DRIVE|VisualSync")
    void ClearAllBindings();

    // Utilidades
    UFUNCTION(BlueprintPure, Category="DRIVE|VisualSync")
    bool HasBinding(int32 EntityId) const;

    UFUNCTION(BlueprintPure, Category="DRIVE|VisualSync")
    AActor* GetBoundActor(int32 EntityId) const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    virtual void TickSync_Native(float DeltaSeconds);

    UFUNCTION(BlueprintImplementableEvent, Category="DRIVE|VisualSync")
    void TickSync_BP(float DeltaSeconds);

    UFUNCTION()
    void HandleDriveTransformUpdated(UDriveEntityComponent* Source, const FTransform& NewTransform);

private:
    int32 FindBindingIndexByEntityId(int32 EntityId) const;
    int32 FindBindingIndexByActor(AActor* InActor) const;
    int32 MakeEntityIdFromActor(AActor* InActor) const;
};