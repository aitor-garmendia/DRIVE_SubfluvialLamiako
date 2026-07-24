#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/DriveSessionTypes.h"

#include "DriveEntityComponent.generated.h"

class UWorldSimulationSubsystem;
class UDriveAgentPreset;

UENUM(BlueprintType)
enum class EDriveEntityLinkState : uint8
{
    Unlinked UMETA(DisplayName="Unlinked"),
    Pending  UMETA(DisplayName="Pending"),
    Linked   UMETA(DisplayName="Linked"),
    Failed   UMETA(DisplayName="Failed")
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDriveEntityLinkStateChanged, EDriveEntityLinkState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDriveEntityLinked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDriveEntityLinkFailed, const FString&, ErrorMessage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDriveEntityRegistered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDriveEntityUnregistered);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDriveTransformUpdated, UDriveEntityComponent*, Source, const FTransform&, NewTransform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDriveEntityPresetApplied);

UCLASS(ClassGroup=(DRIVE), meta=(BlueprintSpawnableComponent))
class DRIVE_UE_API UDriveEntityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDriveEntityComponent();

    // Id  del agente
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent")
    FName AgentId = NAME_None;

    // Nombre para UI/debug
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent")
    FText DisplayName;

    // Rol del agente
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent")
    EDriveAgentType AgentType;    

    // Tags  para búsquedas y filtros
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent")
    TArray<FName> AgentTags;

    UFUNCTION(BlueprintCallable, Category="DRIVE|Agent")
    FText GetAgentLabel() const;

    UFUNCTION(BlueprintCallable, Category="DRIVE|Agent")
    bool HasAgentTag(FName Tag) const;

    // Si el agente puede ser poseído por el PlayerController
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent")
    bool bCanBePossessed = false;

    // Intenta linkear automáticamente en BeginPlay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent")
    bool bAutoLinkOnBeginPlay = true;

    // Auto-aplica el Preset en BeginPlay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent")
    bool bAutoApplyPresetOnBeginPlay = true;

    // El componente emite OnRegistered/OnUnregistered automáticamente
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent")
    bool bAutoRegisterOnBeginPlay = true;

    // Unregister en EndPlay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent")
    bool bAutoUnregisterOnEndPlay = true;

    // Preset de agente
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent|Preset")
    TObjectPtr<UDriveAgentPreset> Preset;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveEntityPresetApplied OnPresetApplied;

    // Aplica el preset
    UFUNCTION(BlueprintCallable, Category="DRIVE|Agent|Preset")
    void ApplyPreset();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Entity")
    EDriveEntityLinkState GetLinkState() const { return LinkState; }

    UFUNCTION(BlueprintCallable, Category="DRIVE|Entity")
    bool IsLinked() const { return LinkState == EDriveEntityLinkState::Linked; }

    UFUNCTION(BlueprintCallable, Category="DRIVE|Entity")
    UWorldSimulationSubsystem* GetWorldSimulationSubsystem() const;

    // Intenta enlazar el owner a DRIVE
    UFUNCTION(BlueprintCallable, Category="DRIVE|Entity")
    bool TryLink();

    // Rompe el enlace y limpia estado local
    UFUNCTION(BlueprintCallable, Category="DRIVE|Entity")
    void Unlink();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Entity")
    FString GetLastError() const { return LastError; }

    // Acepta actualizaciones de transform por sync externo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DRIVE|Agent|Sync")
    bool bAutoSyncTransform = true;

    // Aplica un transform al Owner
    UFUNCTION(BlueprintCallable, Category="DRIVE|Agent|Sync")
    void ApplyDriveTransform(const FTransform& NewTransform, bool bSweep = false);

    // Se dispara cuando se aplica un transform
    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveTransformUpdated OnDriveTransformUpdated;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveEntityLinkStateChanged OnLinkStateChanged;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveEntityLinked OnLinked;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveEntityLinkFailed OnLinkFailed;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveEntityRegistered OnRegistered;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveEntityUnregistered OnUnregistered;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|Entity", meta=(AllowPrivateAccess="true"))
    EDriveEntityLinkState LinkState = EDriveEntityLinkState::Unlinked;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="DRIVE|Entity", meta=(AllowPrivateAccess="true"))
    FString LastError;

private:
    void SetLinkState(EDriveEntityLinkState NewState, const FString& ErrorIfAny = FString());

    void RegisterIfNeeded();
    void UnregisterIfNeeded();
};