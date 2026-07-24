#include "Session/DriveLevelTransitionSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UDriveLevelTransitionSubsystem::SetProgress(float P01, const FText& Phase, bool bActive)
{
    CurrentProgress.Progress01 = FMath::Clamp(P01, 0.f, 1.f);
    CurrentProgress.Phase = Phase;
    CurrentProgress.bIsActive = bActive;
    OnLevelLoadProgress.Broadcast(CurrentProgress);
}

void UDriveLevelTransitionSubsystem::RequestOpenLevel(const FName LevelName)
{
    // Progreso híbrido por fases (válido para OpenLevel)
    SetProgress(0.0f, FText::FromString(TEXT("Preparing transition")), true);
    SetProgress(0.4f, FText::FromString(TEXT("Opening level")), true);

    UGameplayStatics::OpenLevel(GetWorld(), LevelName);
}