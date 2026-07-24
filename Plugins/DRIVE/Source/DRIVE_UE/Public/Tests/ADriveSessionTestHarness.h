#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ADriveSessionTestHarness.generated.h"

class UDriveGlobalConfigAsset;
class UDriveScenarioAsset;
class UWorldSimulationSubsystem;

UCLASS()
class DRIVE_UE_API ADriveSessionTestHarness : public AActor
{
	GENERATED_BODY()

public:
	ADriveSessionTestHarness();

protected:
	virtual void BeginPlay() override;

	// -------- Configuración --------
	UPROPERTY(EditAnywhere, Category="DRIVE Test")
	TObjectPtr<UDriveGlobalConfigAsset> GlobalConfig;

	UPROPERTY(EditAnywhere, Category="DRIVE Test")
	TObjectPtr<UDriveScenarioAsset> ScenarioAsset;

private:
	void RunFullTest();
	void LogState(const FString& StepName, UWorldSimulationSubsystem* Subsystem);
};