#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "DriveGlobalSettings.generated.h"

class UDriveGlobalConfigAsset;
class UDriveScenarioAsset;

UCLASS(config=Game, defaultconfig, meta=(DisplayName="DRIVE Global Settings"))
class DRIVE_UE_API UDriveGlobalSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category="DRIVE|Configuration",
		meta=(AllowedClasses="/Script/DRIVE_Core.DriveGlobalConfigAsset",
			  ToolTip="Asset de configuración global. Si está vacío, DRIVE usa defaults internos."))
	TSoftObjectPtr<UDriveGlobalConfigAsset> GlobalConfigAsset;

	UPROPERTY(EditAnywhere, Config, Category="DRIVE|Scenario",
		meta=(AllowedClasses="/Script/DRIVE_Core.DriveScenarioAsset",
			  ToolTip="Scenario por defecto si no se especifica otro al iniciar."))
	TSoftObjectPtr<UDriveScenarioAsset> DefaultScenarioAsset;
};