#include "BP_FunctionLibrary.h"
#include "Misc/SecureHash.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "WheeledVehiclePawn.h"
#include "ChaosVehicleWheel.h"
#include "RHI.h"
#include "RenderCore.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"

FString UBP_FunctionLibrary::GetSystemHWID()
{
    // read serial numnber from de physic disk C:
    DWORD VolumeSerialNumber = 0;
    GetVolumeInformationW(L"C:\\", NULL, 0, &VolumeSerialNumber, NULL, NULL, NULL, 0);
    
    // convert to hexadecimal
    return FString::Printf(TEXT("%X"), VolumeSerialNumber);
}

FString UBP_FunctionLibrary::GenerateLicenseKey(const FString& InHWID)
{
    FString SecretSalt = TEXT("ClaveDeAccesoSener_24072026"); 
    FString CombinedString = InHWID + SecretSalt;
    
    // return MD5 hash
    return FMD5::HashAnsiString(*CombinedString);
}

void UBP_FunctionLibrary::ChangeWindowMode(EWindowMode::Type WindowMode)
{
    if (GEngine && GEngine->GameViewport)
        if (TSharedPtr<SWindow> MainWindow = GEngine->GameViewport->GetWindow())
            MainWindow->SetWindowMode(WindowMode);
}

EWindowMode::Type UBP_FunctionLibrary::GetWindowMode()
{
    if (GEngine && GEngine->GameViewport)
        if (TSharedPtr<SWindow> MainWindow = GEngine->GameViewport->GetWindow())
            return MainWindow->GetWindowMode();

    return EWindowMode::Windowed;
}

float UBP_FunctionLibrary::GetWheelMaxSteerAngle(UChaosWheeledVehicleMovementComponent* MovementComp, int32 WheelIndex)
{
    if (!MovementComp || !MovementComp->WheelSetups.IsValidIndex(WheelIndex))
        return 0.f;

    const UChaosVehicleWheel* WheelCDO = MovementComp->WheelSetups[WheelIndex].WheelClass.GetDefaultObject();
    return WheelCDO ? WheelCDO->MaxSteerAngle : 0.f;
}

FPerfStats UBP_FunctionLibrary::GetPerfStats()
{
    FPerfStats S;
    S.GameThreadMs   = FPlatformTime::ToMilliseconds(GGameThreadTime);
    S.RenderThreadMs = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    S.GpuMs          = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());

    S.CpuUsagePct = FPlatformTime::GetCPUTime().CPUTimePctRelative;
    const float FrameMs = FApp::GetDeltaTime() * 1000.f;
    const float CpuMs   = FMath::Max(S.GameThreadMs, S.RenderThreadMs);
    S.CpuUsagePct = FrameMs > 0.f ? FMath::Clamp(CpuMs / FrameMs * 100.f, 0.f, 100.f) : 0.f;
    S.GpuUsagePct = FrameMs > 0.f ? FMath::Clamp(S.GpuMs / FrameMs * 100.f, 0.f, 100.f) : 0.f;

    S.DrawCalls   = GNumDrawCallsRHI[0];
    S.Primitives  = GNumPrimitivesDrawnRHI[0]; 

    const FPlatformMemoryStats Mem = FPlatformMemory::GetStats();
    constexpr float GB = 1024.f * 1024.f * 1024.f;
    S.RamUsedGB  = Mem.UsedPhysical / GB;
    S.RamTotalGB = Mem.TotalPhysical / GB;

    S.InputLatencyMs = FPlatformTime::ToMilliseconds(GInputLatencyTime);
    return S;
}

static FVector GetRefPoseBoneLocation(const USkeletalMeshComponent* Mesh, FName Bone)
{
    const USkeletalMesh* Asset = Mesh ? Mesh->GetSkeletalMeshAsset() : nullptr;
    if (!Asset) return FVector::ZeroVector;

    const FReferenceSkeleton& RefSkel = Asset->GetRefSkeleton();
    const int32 Index = RefSkel.FindBoneIndex(Bone);
    if (Index == INDEX_NONE) return FVector::ZeroVector;

    return FAnimationRuntime::GetComponentSpaceTransformRefPose(RefSkel, Index).GetLocation();
}

static float GetTheoreticalMaxSpeedKmh(const UChaosWheeledVehicleMovementComponent* M)
{
    if (!M || M->TransmissionSetup.ForwardGearRatios.Num() == 0 || M->WheelSetups.Num() == 0)
        return 0.f;

    const UChaosVehicleWheel* Wheel = M->WheelSetups[0].WheelClass
        ? M->WheelSetups[0].WheelClass->GetDefaultObject<UChaosVehicleWheel>() : nullptr;
    if (!Wheel) return 0.f;

    // 1) Límite por marchas: última marcha a MaxRPM
    const float TopGear  = M->TransmissionSetup.ForwardGearRatios.Last();
    const float FinalDrv = M->TransmissionSetup.FinalRatio;
    if (FMath::IsNearlyZero(TopGear * FinalDrv)) return 0.f;

    const float WheelRPM     = M->EngineSetup.MaxRPM / (TopGear * FinalDrv);
    const float GearLimitKmh = WheelRPM * 2.f * PI * Wheel->WheelRadius / 60.f * 0.036f;

    // 2) Límite por potencia contra la resistencia del aire
    constexpr float PowerFactor = 0.6f;   // ← el valor a calibrar (ver abajo)
    const float PowerW = M->EngineSetup.MaxTorque * M->EngineSetup.MaxRPM / 9549.f * 1000.f * PowerFactor;

    const float AreaM2   = (M->ChassisWidth / 100.f) * (M->ChassisHeight / 100.f);
    const float DragTerm = 0.5f * 1.225f * M->DragCoefficient * AreaM2;
    const float DragLimitKmh = DragTerm > 0.f ? FMath::Pow(PowerW / DragTerm, 1.f / 3.f) * 3.6f : GearLimitKmh;

    return FMath::Min(GearLimitKmh, DragLimitKmh);
}

static float GetTurningRadiusMeters(const UChaosWheeledVehicleMovementComponent* M, const USkeletalMeshComponent* Mesh)
{
    // Supone 4 ruedas en orden: 0 delantera izq., 1 delantera der., 2 trasera izq., 3 trasera der.
    if (!M || !Mesh || M->WheelSetups.Num() < 4) return 0.f;

    auto WheelPos = [&](int32 i) { return GetRefPoseBoneLocation(Mesh, M->WheelSetups[i].BoneName); };

    const FVector FL = WheelPos(0), FR = WheelPos(1), RL = WheelPos(2), RR = WheelPos(3);

    const float Wheelbase  = FMath::Abs(((FL + FR) * 0.5f).X - ((RL + RR) * 0.5f).X);   // cm
    const float TrackWidth = FMath::Abs(FL.Y - FR.Y);                                     // cm

    const float MaxSteerDeg = M->WheelSetups[0].WheelClass->GetDefaultObject<UChaosVehicleWheel>()->MaxSteerAngle;
    const float SinSteer    = FMath::Sin(FMath::DegreesToRadians(MaxSteerDeg));
    if (SinSteer <= KINDA_SMALL_NUMBER) return 0.f;

    const float RadiusCm = Wheelbase / SinSteer + TrackWidth * 0.5f;
    return RadiusCm / 100.f;   // metros
}

FVehicleStats UBP_FunctionLibrary::ComputeVehicleStats(const UChaosWheeledVehicleMovementComponent* M, const USkeletalMeshComponent* Mesh, float Mu)
{
    FVehicleStats S;
    if (!M || M->Mass <= 0.f) return S;

    const float MassT = M->Mass / 1000.f;

    // Velocidad
    S.MaxSpeedKmh = GetTheoreticalMaxSpeedKmh(M);
    S.SpeedScore  = FMath::GetMappedRangeValueClamped(FVector2D(20.f, 200.f), FVector2D(0.f, 10.f), S.MaxSpeedKmh);

    // Aceleración (potencia/peso, escala logarítmica)
    const float UsableRPM = FMath::Min(M->EngineSetup.MaxRPM, M->TransmissionSetup.ChangeUpRPM);
    const float PowerKW   = M->EngineSetup.MaxTorque * UsableRPM / 9549.f;
    const float KWPerTonne = FMath::Max(PowerKW / MassT, 1.f);
    S.AccelerationScore = FMath::GetMappedRangeValueClamped(
        FVector2D(FMath::LogX(10.f, 5.f), FMath::LogX(10.f, 150.f)), FVector2D(0.f, 10.f),
        FMath::LogX(10.f, KWPerTonne));

    // Maniobrabilidad (radio de giro, invertido)
    const float RadiusM = GetTurningRadiusMeters(M, Mesh);
    S.ManeuverabilityScore = FMath::GetMappedRangeValueClamped(FVector2D(13.f, 4.5f), FVector2D(0.f, 10.f), RadiusM);

    // Frenada (km/h que pierde cada segundo)
    float BrakeForceN = 0.f;
    for (const FChaosWheelSetup& Setup : M->WheelSetups)
    {
        const UChaosVehicleWheel* W = Setup.WheelClass->GetDefaultObject<UChaosVehicleWheel>();
        BrakeForceN += W->MaxBrakeTorque / (W->WheelRadius / 100.f);
    }
    const float Decel = FMath::Min(BrakeForceN / M->Mass, Mu * 9.81f);   // m/s²
    S.BrakeKmhPerSec  = Decel * 3.6f;

    return S;
}

FVehicleStats UBP_FunctionLibrary::ComputeVehicleStatsFromClass(TSubclassOf<AWheeledVehiclePawn> VehicleClass, float Mu)
{
    if (!VehicleClass) return FVehicleStats();

    const AWheeledVehiclePawn* CDO = VehicleClass->GetDefaultObject<AWheeledVehiclePawn>();
    const auto* Movement = Cast<UChaosWheeledVehicleMovementComponent>(CDO->GetVehicleMovementComponent());
    const USkeletalMeshComponent* Mesh = CDO->GetMesh();

    return ComputeVehicleStats(Movement, Mesh, Mu);
}