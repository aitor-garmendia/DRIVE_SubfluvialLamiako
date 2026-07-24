#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DriveBlueprintLibrary.generated.h"

class UWorldSimulationSubsystem;
class UDriveScenarioAsset;

UCLASS()
class DRIVE_UE_API UDriveBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="DRIVE|Subsystem", meta=(WorldContext="WorldContextObject"))
	static UWorldSimulationSubsystem* GetWorldSimulationSubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="DRIVE|Simulation", meta=(WorldContext="WorldContextObject"))
	static bool IsDriveSimulationRunning(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation", meta=(WorldContext="WorldContextObject"))
	static void DriveSim_Start(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation", meta=(WorldContext="WorldContextObject"))
	static void DriveSim_Pause(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation", meta=(WorldContext="WorldContextObject"))
	static void DriveSim_Stop(const UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category="DRIVE|Scenario", meta=(WorldContext="WorldContextObject"))
	static bool DriveSim_LoadScenario(const UObject* WorldContextObject, const UDriveScenarioAsset* ScenarioAsset);

	UFUNCTION(BlueprintCallable, Category="DRIVE|Clock", meta=(WorldContext="WorldContextObject"))
	static void DriveSim_SetTimeScale(const UObject* WorldContextObject, float TimeScale);

	UFUNCTION(BlueprintPure, Category="DRIVE|Clock", meta=(WorldContext="WorldContextObject"))
	static float DriveSim_GetTimeScale(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation", meta=(WorldContext="WorldContextObject"))
	static bool DriveSim_InitializeAndBuildDefaults(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="DRIVE|Simulation", meta=(WorldContext="WorldContextObject"))
	static bool DriveSim_Initialize(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="DRIVE|Scenario", meta=(WorldContext="WorldContextObject"))
	static bool DriveSim_HasScenarioLoaded(const UObject* WorldContextObject);
};