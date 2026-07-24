#include "Product/DriveVisualSyncManagerBase.h"

#include "Blueprint/Components/DriveEntityComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "Logging/DriveLog.h"

ADriveVisualSyncManagerBase::ADriveVisualSyncManagerBase()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADriveVisualSyncManagerBase::BeginPlay()
{
    Super::BeginPlay();

    if (bDebug)
    {
        DRIVE_LOG_MANAGER(Log, TEXT("[DRIVE][VisualSync] BeginPlay. Tick=%s Smoothing=%s"), bTickEnabled ? TEXT("true") : TEXT("false"), bEnableSmoothing ? TEXT("true") : TEXT("false"));
    }
}

void ADriveVisualSyncManagerBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bTickEnabled)
        return;

    TickSync_Native(DeltaSeconds);
    TickSync_BP(DeltaSeconds); 
}

void ADriveVisualSyncManagerBase::TickSync_Native(float DeltaSeconds)
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

    for (int32 i = Bindings.Num() - 1; i >= 0; --i)
    {
        FDriveVisualBinding& B = Bindings[i];

        AActor* BoundActor = B.Actor.Get();
        if (!BoundActor)
        {
            if (bDebug)
            {
                DRIVE_LOG_MANAGER(Log, TEXT("[DRIVE][VisualSync] Removing binding EntityId=%d (Actor invalid)"), B.EntityId);
            }
            Bindings.RemoveAtSwap(i);
            continue;
        }

        if (!B.bHasTarget)
        {
            continue; 
        }

        const FTransform Current = BoundActor->GetActorTransform();
        const FTransform& Target = B.TargetTransform;

        const FVector CurrentLoc = Current.GetLocation();
        const FVector TargetLoc  = Target.GetLocation();
        const float Dist = FVector::Dist(CurrentLoc, TargetLoc);

        // Snap si está muy lejos 
        if (SnapDistance > 0.f && Dist > SnapDistance)
        {
            BoundActor->SetActorTransform(Target, false, nullptr, ETeleportType::TeleportPhysics);
            B.LastUpdateTime = Now;
            continue;
        }

        // Sin smoothing 
        if (!bEnableSmoothing || (LocationInterpSpeed <= 0.f && RotationInterpSpeed <= 0.f))
        {
            BoundActor->SetActorTransform(Target, false);
            B.LastUpdateTime = Now;
            continue;
        }

        // Interpolación
        FVector NewLoc = CurrentLoc;
        if (LocationInterpSpeed > 0.f)
        {
            NewLoc = FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaSeconds, LocationInterpSpeed);
        }

        const FQuat CurrentRot = Current.GetRotation();
        const FQuat TargetRot  = Target.GetRotation();
        FQuat NewRot = CurrentRot;

        if (RotationInterpSpeed > 0.f)
        {
            NewRot = FMath::QInterpTo(CurrentRot, TargetRot, DeltaSeconds, RotationInterpSpeed);
        }

        const float ScaleSpeed = (LocationInterpSpeed > 0.f) ? LocationInterpSpeed : 10.f;
        const FVector NewScale = FMath::VInterpTo(Current.GetScale3D(), Target.GetScale3D(), DeltaSeconds, ScaleSpeed);

        FTransform Smoothed;
        Smoothed.SetLocation(NewLoc);
        Smoothed.SetRotation(NewRot);
        Smoothed.SetScale3D(NewScale);

        BoundActor->SetActorTransform(Smoothed, false);
        B.LastUpdateTime = Now;
    }
}

bool ADriveVisualSyncManagerBase::RegisterActor(AActor* InActor, int32 EntityId)
{
    if (!InActor)
    {
        if (bDebug) DRIVE_LOG_MANAGER(Log, TEXT("[DRIVE][VisualSync] RegisterActor: InActor null"));
        return false;
    }

    UDriveEntityComponent* EntityComp = InActor->FindComponentByClass<UDriveEntityComponent>();
    if (!EntityComp)
    {
        if (bDebug) DRIVE_LOG_MANAGER(Log, TEXT("[DRIVE][VisualSync] RegisterActor: %s has no DriveEntityComponent"), *InActor->GetName());
        return false;
    }

    const int32 EffectiveId = (EntityId >= 0) ? EntityId : MakeEntityIdFromActor(InActor);

    int32 Idx = FindBindingIndexByEntityId(EffectiveId);
    if (Idx == INDEX_NONE)
    {
        FDriveVisualBinding NewB;
        NewB.EntityId = EffectiveId;
        NewB.Actor = InActor;
        NewB.EntityComponent = EntityComp;
        NewB.TargetTransform = InActor->GetActorTransform();
        NewB.bHasTarget = false;
        NewB.LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
        Bindings.Add(MoveTemp(NewB));

        Idx = Bindings.Num() - 1;
    }
    else
    {
        FDriveVisualBinding& B = Bindings[Idx];
        B.Actor = InActor;
        B.EntityComponent = EntityComp;
    }

    EntityComp->OnDriveTransformUpdated.RemoveDynamic(this, &ADriveVisualSyncManagerBase::HandleDriveTransformUpdated);
    EntityComp->OnDriveTransformUpdated.AddDynamic(this, &ADriveVisualSyncManagerBase::HandleDriveTransformUpdated);

    if (bDebug)
    {
        DRIVE_LOG_MANAGER(Log, TEXT("[DRIVE][VisualSync] Registered EntityId=%d -> %s"), EffectiveId, *InActor->GetName());
    }

    return true;
}

bool ADriveVisualSyncManagerBase::UnregisterByEntityId(int32 EntityId)
{
    const int32 Idx = FindBindingIndexByEntityId(EntityId);
    if (Idx == INDEX_NONE)
        return false;

    if (UDriveEntityComponent* Comp = Bindings[Idx].EntityComponent.Get())
    {
        Comp->OnDriveTransformUpdated.RemoveDynamic(this, &ADriveVisualSyncManagerBase::HandleDriveTransformUpdated);
    }

    Bindings.RemoveAtSwap(Idx);
    return true;
}

bool ADriveVisualSyncManagerBase::UnregisterActor(AActor* InActor)
{
    const int32 Idx = FindBindingIndexByActor(InActor);
    if (Idx == INDEX_NONE)
        return false;

    return UnregisterByEntityId(Bindings[Idx].EntityId);
}

void ADriveVisualSyncManagerBase::ClearAllBindings()
{
    for (FDriveVisualBinding& B : Bindings)
    {
        if (UDriveEntityComponent* Comp = B.EntityComponent.Get())
        {
            Comp->OnDriveTransformUpdated.RemoveDynamic(this, &ADriveVisualSyncManagerBase::HandleDriveTransformUpdated);
        }
    }

    Bindings.Reset();
}

bool ADriveVisualSyncManagerBase::HasBinding(int32 EntityId) const
{
    return FindBindingIndexByEntityId(EntityId) != INDEX_NONE;
}

AActor* ADriveVisualSyncManagerBase::GetBoundActor(int32 EntityId) const
{
    const int32 Idx = FindBindingIndexByEntityId(EntityId);
    return (Idx != INDEX_NONE) ? Bindings[Idx].Actor.Get() : nullptr;
}


void ADriveVisualSyncManagerBase::HandleDriveTransformUpdated(UDriveEntityComponent* Source, const FTransform& NewTransform)
{
    if (!Source)
        return;

    AActor* SourceOwner = Source->GetOwner();
    if (!SourceOwner)
        return;

    const int32 Idx = FindBindingIndexByActor(SourceOwner);
    if (Idx == INDEX_NONE)
    {
        if (bDebug)
        {
            DRIVE_LOG_MANAGER(Log, TEXT("[DRIVE][VisualSync] Transform for unregistered Actor: %s"), *SourceOwner->GetName());
        }
        return;
    }

    FDriveVisualBinding& B = Bindings[Idx];
    B.TargetTransform = NewTransform;
    B.bHasTarget = true;
    B.LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}

int32 ADriveVisualSyncManagerBase::FindBindingIndexByEntityId(int32 EntityId) const
{
    for (int32 i = 0; i < Bindings.Num(); ++i)
    {
        if (Bindings[i].EntityId == EntityId)
            return i;
    }
    return INDEX_NONE;
}

int32 ADriveVisualSyncManagerBase::FindBindingIndexByActor(AActor* InActor) const
{
    if (!InActor)
        return INDEX_NONE;

    for (int32 i = 0; i < Bindings.Num(); ++i)
    {
        if (Bindings[i].Actor.Get() == InActor)
            return i;
    }
    return INDEX_NONE;
}

int32 ADriveVisualSyncManagerBase::MakeEntityIdFromActor(AActor* InActor) const
{
    return InActor ? GetTypeHash(InActor->GetFName()) : -1;
}