#include "Systems/FDriveMovementSystem.h"
#include "Entities/Queries/DriveEntityQuery.h"
#include "Logging/DriveLog.h"

using namespace Drive::Query;

bool FDriveMovementSystem::Initialize(const FDriveSystemContext& Context)
{
    if (!FDriveSystemBase::Initialize(Context))
    {
        return false;
    }

	SeedDemoDataIfNeeded();
	DRIVE_LOG_SYSTEMS(Log, TEXT("[DRIVE][MovementSystem] Initialized"));
	return true;
}

void FDriveMovementSystem::TickFixed(float FixedDt)
{
	SeedDemoDataIfNeeded();
	auto View = MakeView(Entities, TransformStore, VelocityStore);

	int32 UpdatedCount = 0;

	View.ForEach([&](FDriveEntityId EntityId, FTransformComp& Transform, FVelocityComp& Velocity)
	{
		Transform.Position += Velocity.Value * FixedDt;
		UpdatedCount++;
	});

	DRIVE_LOG_SYSTEMS(Verbose, TEXT("[DRIVE][MovementSystem] TickFixed dt=%.4f Updated=%d"), FixedDt, UpdatedCount);
}

void FDriveMovementSystem::Shutdown()
{
	Entities.Reset();
	TransformStore.Data.Reset();
	VelocityStore.Data.Reset();
	bSeeded = false;

	DRIVE_LOG_SYSTEMS(Log, TEXT("[DRIVE][MovementSystem] Shutdown"));
	FDriveSystemBase::Shutdown();
}

void FDriveMovementSystem::SeedDemoDataIfNeeded()
{
	if (bSeeded)
	{
		return;
	}

	Entities = { 1, 2, 3 };

	TransformStore.Data.Add(1, FTransformComp{ FVector(0, 0, 0) });
	VelocityStore.Data.Add(1, FVelocityComp{ FVector(100, 0, 0) });

	TransformStore.Data.Add(2, FTransformComp{ FVector(0, 0, 0) });

	TransformStore.Data.Add(3, FTransformComp{ FVector(0, 0, 0) });
	VelocityStore.Data.Add(3, FVelocityComp{ FVector(0, 50, 0) });

	bSeeded = true;
	DRIVE_LOG_SYSTEMS(Log, TEXT("[DRIVE][MovementSystem] Seeded demo data: Entities=%d"), Entities.Num());
}
