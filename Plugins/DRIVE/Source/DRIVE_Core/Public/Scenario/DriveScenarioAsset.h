#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Scenario/DriveScenarioDefinition.h"
#include "DriveScenarioAsset.generated.h"

UCLASS(BlueprintType)
class DRIVE_CORE_API UDriveScenarioAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Scenario")
	FName ScenarioId = TEXT("DefaultScenario");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DRIVE|Scenario")
	TArray<FName> EnabledSystems;

	bool Validate(FString& OutError) const;
	FDriveScenarioDefinition ToDefinition() const;
};
