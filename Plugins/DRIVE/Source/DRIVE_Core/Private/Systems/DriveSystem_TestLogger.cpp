#include "Systems/DriveSystem_TestLogger.h"
#include "Logging/DriveLog.h"

FName FDriveSystem_TestLogger::GetSystemId() const { return TEXT("TestLogger"); }
int32 FDriveSystem_TestLogger::GetInitOrder() const { return 10; }
int32 FDriveSystem_TestLogger::GetFixedTickOrder() const { return 10; }

bool FDriveSystem_TestLogger::Initialize(const FDriveSystemContext& Context)
{
	const bool bOk = FDriveSystemBase::Initialize(Context);
	DRIVE_LOG_SYSTEMS(Log, TEXT("[DRIVE][System:TestLogger] Initialize"));
	return bOk;
}

void FDriveSystem_TestLogger::TickFixed(float FixedDt)
{
	DRIVE_LOG_SYSTEMS(VeryVerbose, TEXT("[DRIVE][System:TestLogger] TickFixed dt=%.4f"), FixedDt);
}
