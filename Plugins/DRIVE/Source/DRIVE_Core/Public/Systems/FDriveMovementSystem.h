#pragma once

#include "CoreMinimal.h"
#include "Systems/DriveSystemBase.h"

class DRIVE_CORE_API FDriveMovementSystem : public FDriveSystemBase
{
public:
	virtual FName GetSystemId() const override { return TEXT("Drive.Movement"); }

	virtual int32 GetInitOrder() const override { return 10; }
	virtual int32 GetFixedTickOrder() const override { return 10; }

	virtual bool Initialize(const FDriveSystemContext& Context) override;
	virtual void TickFixed(float FixedDt) override;
	virtual void Shutdown() override;

private:
	using FDriveEntityId = int32;

	struct FTransformComp
	{
		FVector Position = FVector::ZeroVector;
	};

	struct FVelocityComp
	{
		FVector Value = FVector::ZeroVector;
	};

	template<typename TComp>
	struct TSimpleStore
	{
		TMap<FDriveEntityId, TComp> Data;

		bool Has(FDriveEntityId Id) const { return Data.Contains(Id); }

		TComp& Get(FDriveEntityId Id) { return Data.FindChecked(Id); }
		const TComp& Get(FDriveEntityId Id) const { return Data.FindChecked(Id); }
	};

	TArray<FDriveEntityId> Entities;

	TSimpleStore<FTransformComp> TransformStore;
	TSimpleStore<FVelocityComp> VelocityStore;

	bool bSeeded = false;
	void SeedDemoDataIfNeeded();
};
