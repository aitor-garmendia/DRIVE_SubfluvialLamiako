#include "BP_FunctionLibrary.h"
#include "Misc/SecureHash.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"

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