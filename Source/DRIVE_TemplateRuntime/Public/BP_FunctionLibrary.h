#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/TextureCubeArray.h"
#include "Engine/Texture2DArray.h"
#include "BP_FunctionLibrary.generated.h"

class UChaosWheeledVehicleMovementComponent;

USTRUCT(BlueprintType)
struct FPerfStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) float GameThreadMs = 0.f;   // CPU: lógica
    UPROPERTY(BlueprintReadOnly) float RenderThreadMs = 0.f; // CPU: preparar el dibujo
    UPROPERTY(BlueprintReadOnly) float CpuUsagePct = 0.f;    // % de CPU que usa la aplicación
    UPROPERTY(BlueprintReadOnly) float GpuMs = 0.f;          // GPU
    UPROPERTY(BlueprintReadOnly) float GpuUsagePct = 0.f;    // % de GPU que usa la aplicación
    UPROPERTY(BlueprintReadOnly) int32 DrawCalls   = 0;
    UPROPERTY(BlueprintReadOnly) int32 Primitives  = 0;
    UPROPERTY(BlueprintReadOnly) float RamUsedGB = 0.f;      // RAM que usa la aplicación
    UPROPERTY(BlueprintReadOnly) float RamTotalGB = 0.f;     // RAM total del equipo
    UPROPERTY(BlueprintReadOnly) float InputLatencyMs = 0.f;
};

USTRUCT(BlueprintType)
struct FVehicleStats
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) float MaxSpeedKmh = 0.f;
    UPROPERTY(BlueprintReadOnly) float BrakeKmhPerSec = 0.f;
    UPROPERTY(BlueprintReadOnly) float SpeedScore = 0.f;           // 0–10
    UPROPERTY(BlueprintReadOnly) float AccelerationScore = 0.f;    // 0–10
    UPROPERTY(BlueprintReadOnly) float ManeuverabilityScore = 0.f; // 0–10
};

UCLASS()
class DRIVE_TEMPLATERUNTIME_API UBP_FunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Licencia")
    static FString GetSystemHWID();

    UFUNCTION(BlueprintCallable, Category = "Licencia")
    static FString GenerateLicenseKey(const FString& InHWID);

	UFUNCTION(BlueprintCallable, Category = "Window")
    static void ChangeWindowMode(EWindowMode::Type WindowMode);

	UFUNCTION(BlueprintCallable, Category = "Window")
    static EWindowMode::Type GetWindowMode();

    UFUNCTION(BlueprintPure, Category = "Vehicle")
    static float GetWheelMaxSteerAngle(UChaosWheeledVehicleMovementComponent* MovementComp, int32 WheelIndex = 0);

    UFUNCTION(BlueprintPure, Category = "Performance")
    static FPerfStats GetPerfStats();

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    static FVehicleStats ComputeVehicleStatsFromClass(TSubclassOf<AWheeledVehiclePawn> VehicleClass, float Mu = 0.8f);
    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    static FVehicleStats ComputeVehicleStats(const UChaosWheeledVehicleMovementComponent* M, const USkeletalMeshComponent* Mesh, float Mu = 0.8f);
};
