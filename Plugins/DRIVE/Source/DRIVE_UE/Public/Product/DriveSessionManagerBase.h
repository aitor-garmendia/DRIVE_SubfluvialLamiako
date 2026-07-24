#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DriveSessionManagerBase.generated.h"

class UWorldSimulationSubsystem;
class UDriveGlobalConfigAsset;
class UDriveScenarioAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDriveSessionEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDriveScenarioLoadedEvent, FName, ScenarioId);

UCLASS(Abstract, Blueprintable)
class DRIVE_UE_API ADriveSessionManagerBase : public AActor
{
    GENERATED_BODY()

public:
    ADriveSessionManagerBase();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    bool EnsureSubsystem();

private:
    UPROPERTY(Transient)
    TObjectPtr<UWorldSimulationSubsystem> CachedSubsystem = nullptr;

public:

    // CONFIGURACIÓN DE PRODUCTO
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Session")
    TObjectPtr<const UDriveGlobalConfigAsset> GlobalConfig = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Session")
    TObjectPtr<const UDriveScenarioAsset> ScenarioAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Session")
    bool bAutoInitialize = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Session")
    bool bAutoLoadScenario = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Session")
    bool bAutoStart = false;

    // API PÚBLICA PARA BLUEPRINT
    UFUNCTION(BlueprintCallable, Category="DRIVE|Session")
    bool InitializeSession();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Session")
    bool LoadScenario();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Session")
    bool StartSession();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Session")
    void PauseSession();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Session")
    void ResumeSession();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Session")
    void StopSession();

    UFUNCTION(BlueprintCallable, Category="DRIVE|Session")
    int32 StepManual(int32 NumSteps);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Session")
    bool ResetSession(bool bReloadScenario);

    UFUNCTION(BlueprintPure, Category="DRIVE|Session")
    UWorldSimulationSubsystem* GetDriveSubsystem() const { return CachedSubsystem; }

    
    // EVENTOS PARA BP
    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveSessionEvent OnSessionStarted;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveSessionEvent OnSessionPaused;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveSessionEvent OnSessionResumed;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveSessionEvent OnSessionStopped;

    UPROPERTY(BlueprintAssignable, Category="DRIVE|Events")
    FDriveScenarioLoadedEvent OnScenarioLoaded;

private:

    void BindSubsystemEvents();
    void UnbindSubsystemEvents();

    UFUNCTION()
    void HandleSimStarted();

    UFUNCTION()
    void HandleSimPaused();

    UFUNCTION()
    void HandleSimResumed();

    UFUNCTION()
    void HandleSimStopped();

    UFUNCTION()
    void HandleScenarioLoaded(FName ScenarioId);
};