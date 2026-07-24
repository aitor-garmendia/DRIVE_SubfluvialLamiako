#pragma once

#include "Systems/DriveSystemBase.h"
#include "ECS/DriveEcsTestComponent.h"
#include "ECS/DriveEcsWorld.h"
#include "Simulation/Components/DriveCommonComponents.h"
#include "Logging/DriveLog.h"

class FDriveDummySystem : public FDriveSystemBase
{
public:
	virtual FName GetSystemId() const override
	{
		return TEXT("DummySystem");
	}

    virtual int32 GetInitOrder() const override { return 0; }
    virtual int32 GetFixedTickOrder() const override { return 0; }

    virtual bool Initialize(const FDriveSystemContext& Context) override
	{
		const bool bOk = FDriveSystemBase::Initialize(Context);

		StepCounter = 0;
		AccumulatedTime = 0.0f;

        TransformComp.Transform = FTransform::Identity;
		TransformComp.SetLocation(FVector(0.f, 0.f, 0.f));

		VelocityComp.Linear = FVector(100.f, 0.f, 0.f); // 100 cm/s hacia +X
		TagsComp.Tags.Reset();
		TagsComp.AddTag(TEXT("Debug"));
		TagsComp.AddTag(TEXT("TestEntity"));

		DRIVE_LOG_SYSTEMS(Log, TEXT("[DummySystem] Initialize | Config=%s Clock=%s Runtime=%s Bus=%s | Tags=%d"),
			Context.GlobalConfig ? TEXT("OK") : TEXT("NULL"), Context.Clock ? TEXT("OK") : TEXT("NULL"), Context.Runtime ? TEXT("OK") : TEXT("NULL"), Context.Bus ? TEXT("OK") : TEXT("NULL"), TagsComp.Tags.Num()
		);

		if (!Context.SimState)
		{
			DRIVE_LOG_SYSTEMS(Error, TEXT("[DummySystem] Context.SimState is NULL"));
			return false;
		}

		FDriveEntity E = Context.SimState->CreateEntity();
		DRIVE_LOG_SYSTEMS(Log, TEXT("[DRIVE][Dummy] Created Entity Id=%d Gen=%d Alive=%d"), E.Id, E.Generation, Context.SimState->GetAliveCount());

		Context.SimState->DestroyEntity(E);
		DRIVE_LOG_SYSTEMS(Log, TEXT("[DRIVE][Dummy] Destroyed Entity Alive=%d"), Context.SimState->GetAliveCount());

    	return FDriveSystemBase::Initialize(Context);
	}

    virtual void TickFixed(float FixedDt) override
    {
        ++StepCounter;
		AccumulatedTime += FixedDt;

		if (!Ctx.EcsWorld)
		{
			DRIVE_LOG_SYSTEMS(Error, TEXT("[DRIVE][DummySystem] No ECS World in context"));
			return;
		}

		static bool bOnce = false;
		if (!bOnce)
		{
			bOnce = true;
			DRIVE_LOG_SYSTEMS(Log, TEXT("[DRIVE][DummySystem] Running (deferred ops ready)"));
		}

		TransformComp.AddWorldOffset(VelocityComp.Linear * FixedDt);
		if (StepCounter % LogEveryNSteps == 0)
		{
			const FVector L = TransformComp.GetLocation();
			DRIVE_LOG_SYSTEMS(Verbose, TEXT("[DummySystem] TickFixed step=%d t=%.2fs dt=%.3f Loc=(%.1f, %.1f, %.1f) HasTag(Debug)=%d"),
				StepCounter, AccumulatedTime, FixedDt,
				L.X, L.Y, L.Z,
				TagsComp.HasTag(TEXT("Debug")) ? 1 : 0
			);
		}
    }

	virtual void Shutdown() override
	{
		DRIVE_LOG_SYSTEMS(Log, TEXT("[DummySystem] Shutdown | Steps=%d | t=%.2fs"), StepCounter, AccumulatedTime);
		FDriveSystemBase::Shutdown();
	}

private:
	FDriveTransformComponent TransformComp;
	FDriveVelocityComponent VelocityComp;
	FDriveTagsComponent TagsComp;

	int32 StepCounter = 0;
	float AccumulatedTime = 0.0f;
	static constexpr int32 LogEveryNSteps = 60;
};
