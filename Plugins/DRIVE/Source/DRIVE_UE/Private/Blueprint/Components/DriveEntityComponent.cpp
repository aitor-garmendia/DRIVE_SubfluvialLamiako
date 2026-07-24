#include "Blueprint/Components/DriveEntityComponent.h"
#include "UWorldSimulationSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Blueprint/DataAssets/DriveAgentPreset.h"

UDriveEntityComponent::UDriveEntityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    AgentType = static_cast<EDriveAgentType>(0);
}

void UDriveEntityComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoApplyPresetOnBeginPlay && Preset)
    {
        ApplyPreset();
    }

    if (bAutoRegisterOnBeginPlay)
    {
        RegisterIfNeeded();
    }

    if (bAutoLinkOnBeginPlay)
    {
        TryLink();
    }
}

void UDriveEntityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bAutoUnregisterOnEndPlay)
    {
        UnregisterIfNeeded();
    }

    Unlink();
    Super::EndPlay(EndPlayReason);
}

FText UDriveEntityComponent::GetAgentLabel() const
{
    if (!DisplayName.IsEmpty())
    {
        return DisplayName;
    }

    if (AgentId != NAME_None)
    {
        return FText::FromName(AgentId);
    }

    const AActor* Owner = GetOwner();
    return Owner ? FText::FromString(Owner->GetName()) : FText::FromString(TEXT("DriveAgent"));
}

bool UDriveEntityComponent::HasAgentTag(FName Tag) const
{
    return Tag != NAME_None && AgentTags.Contains(Tag);
}

UWorldSimulationSubsystem* UDriveEntityComponent::GetWorldSimulationSubsystem() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    return World->GetSubsystem<UWorldSimulationSubsystem>();
}

void UDriveEntityComponent::RegisterIfNeeded()
{
    OnRegistered.Broadcast();
}

void UDriveEntityComponent::UnregisterIfNeeded()
{
    OnUnregistered.Broadcast();
}

void UDriveEntityComponent::ApplyPreset()
{
    if (!Preset) return;

    AgentType = Preset->AgentType;
    bCanBePossessed = Preset->bCanBePossessed;
    bAutoLinkOnBeginPlay = Preset->bAutoLinkOnBeginPlay;
    bAutoRegisterOnBeginPlay = Preset->bAutoRegisterOnBeginPlay;
    bAutoSyncTransform = Preset->bAutoSyncTransform;

    OnPresetApplied.Broadcast();
}

void UDriveEntityComponent::ApplyDriveTransform(const FTransform& NewTransform, bool bSweep)
{
    if (!bAutoSyncTransform)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    Owner->SetActorTransform(NewTransform, bSweep);
    OnDriveTransformUpdated.Broadcast(this, NewTransform);
}

void UDriveEntityComponent::SetLinkState(EDriveEntityLinkState NewState, const FString& ErrorIfAny)
{
    LinkState = NewState;
    LastError = ErrorIfAny;

    OnLinkStateChanged.Broadcast(LinkState);

    if (LinkState == EDriveEntityLinkState::Linked)
    {
        OnLinked.Broadcast();
    }
    else if (LinkState == EDriveEntityLinkState::Failed)
    {
        OnLinkFailed.Broadcast(LastError);
    }
}

bool UDriveEntityComponent::TryLink()
{
    if (LinkState == EDriveEntityLinkState::Linked)
    {
        return true;
    }

    SetLinkState(EDriveEntityLinkState::Pending);

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        SetLinkState(EDriveEntityLinkState::Failed, TEXT("DriveEntityComponent has no Owner."));
        return false;
    }

    UWorldSimulationSubsystem* Subsystem = GetWorldSimulationSubsystem();
    if (!Subsystem)
    {
        SetLinkState(EDriveEntityLinkState::Failed, TEXT("WorldSimulationSubsystem not found in this World."));
        return false;
    }

    SetLinkState(EDriveEntityLinkState::Linked);
    return true;
}

void UDriveEntityComponent::Unlink()
{
    if (LinkState == EDriveEntityLinkState::Unlinked)
    {
        return;
    }

    SetLinkState(EDriveEntityLinkState::Unlinked);
}