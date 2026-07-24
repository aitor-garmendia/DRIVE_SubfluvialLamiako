#pragma once
#include "CoreMinimal.h"

class DRIVE_UE_API FDriveDebugCommands
{
public:
    static void Register();
    static void Unregister();

private:
    static void Cmd_Help(const TArray<FString>& Args);
    static void Cmd_Clock(const TArray<FString>& Args);
    static void Cmd_Runtime(const TArray<FString>& Args);
    static void Cmd_State(const TArray<FString>& Args);
    static void Cmd_All(const TArray<FString>& Args);
};