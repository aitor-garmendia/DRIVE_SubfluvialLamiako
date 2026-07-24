#include "Actors/DriveFreeCamera.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"

ADriveFreeCamera::ADriveFreeCamera()
{
}

void ADriveFreeCamera::ChangeViewMode(EViewModeIndex ViewMode) const
{
    if (UGameViewportClient* GameViewport = GetGameViewport())
        GameViewport->SetViewMode(ViewMode);
}

int ADriveFreeCamera::GetCurrentViewMode() const
{
    if (UGameViewportClient* GameViewport = GetGameViewport())
    {
        EViewModeIndex ViewMode = static_cast<EViewModeIndex>(GameViewport->ViewModeIndex);

        switch (ViewMode)
        {
            case VMI_Lit:            return 1;
            case VMI_Unlit:          return 2;
            case VMI_BrushWireframe: return 3;
            case VMI_LightingOnly:   return 4;
            default:                 return 0;
        }   
    }
    return 0;
}

UGameViewportClient* ADriveFreeCamera::GetGameViewport() const
{
    if (UWorld* World = GetWorld())
        return World->GetGameViewport();
    return nullptr;
}