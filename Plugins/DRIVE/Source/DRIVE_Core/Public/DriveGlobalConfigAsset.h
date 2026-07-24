#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DriveGlobalConfigAsset.generated.h"

UENUM(BlueprintType)
enum class EDriveInitPolicy : uint8
{
	Manual      UMETA(DisplayName="Manual"),
	AutoStart   UMETA(DisplayName="AutoStart"),
};

UCLASS(BlueprintType)
class DRIVE_CORE_API UDriveGlobalConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Tiempo base del framework ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DRIVE|Time", meta=(ClampMin="0.0001"))
	float FixedDeltaTime = 1.0f / 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DRIVE|Time", meta=(ClampMin="0.0", ClampMax="100.0"))
	float DefaultTimeScale = 1.0f;

	// Evita spirals si hay hitch grande (lo usará T4/T9)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DRIVE|Time", meta=(ClampMin="1", ClampMax="120"))
	int32 MaxFixedStepsPerFrame = 8;

	// --- Límites globales ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DRIVE|Limits", meta=(ClampMin="0"))
	int32 MaxAgents = 500;

	// --- Debug / flags ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DRIVE|Debug")
	bool bDebugLogs = true;

	// --- Políticas ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DRIVE|Policy")
	bool bAllowInPIE = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DRIVE|Policy")
	EDriveInitPolicy InitPolicy = EDriveInitPolicy::Manual;

	// Validación mínima (la llamará el host)
	bool IsValidConfig(FString& OutError) const
	{
		if (FixedDeltaTime <= 0.0f)
		{
			OutError = TEXT("FixedDeltaTime must be > 0.");
			return false;
		}
		if (MaxFixedStepsPerFrame <= 0)
		{
			OutError = TEXT("MaxFixedStepsPerFrame must be >= 1.");
			return false;
		}
		if (MaxAgents < 0)
		{
			OutError = TEXT("MaxAgents must be >= 0.");
			return false;
		}
		return true;
	}
};