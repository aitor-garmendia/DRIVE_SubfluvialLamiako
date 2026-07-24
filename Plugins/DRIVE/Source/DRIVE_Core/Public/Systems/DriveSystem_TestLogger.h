#pragma once

#include "CoreMinimal.h"
#include "Systems/DriveSystemBase.h"

class DRIVE_CORE_API FDriveSystem_TestLogger : public FDriveSystemBase
{
public:
	virtual FName GetSystemId() const override;
	virtual int32 GetInitOrder() const override;
	virtual int32 GetFixedTickOrder() const override;

	virtual bool Initialize(const FDriveSystemContext& Context) override;
	virtual void TickFixed(float FixedDt) override;
};
