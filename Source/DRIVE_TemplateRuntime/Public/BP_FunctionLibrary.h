#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BP_FunctionLibrary.generated.h"

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
};
