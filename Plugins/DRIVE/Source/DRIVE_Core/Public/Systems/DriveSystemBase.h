#pragma once

#include "CoreMinimal.h"
#include "Systems/DriveSystemContext.h"
#include "Logging/DriveLog.h"

class DRIVE_CORE_API FDriveSystemBase : public TSharedFromThis<FDriveSystemBase>
{
public:
	virtual ~FDriveSystemBase() = default;

	virtual FName GetSystemId() const = 0;
	virtual FString GetDebugName() const { return GetSystemId().ToString(); }
	
	virtual void GetDependencies(TArray<FName>& OutDeps) const { OutDeps.Reset(); }
	virtual int32 GetInitOrder() const { return 0; }
	virtual int32 GetFixedTickOrder() const { return 0; }

	bool IsInitialized() const { return bInitialized; }

	virtual bool Initialize(const FDriveSystemContext& Context)
	{
		if (bInitialized)
		{
			DRIVE_LOG_SYSTEMS(Verbose, TEXT("[DRIVE][SYS:%s] Initialize called twice (ignored)"), *GetDebugName());
			return true;
		}

		// Validación mínima para el pipeline de la Tarea 5
		if (Context.Bus == nullptr)
		{
			DRIVE_LOG_SYSTEMS(Error, TEXT("[DRIVE][SYS:%s] Initialize failed: Context.Bus is null"), *GetDebugName());
			return false;
		}

		Ctx = Context;
		bInitialized = true;

		DRIVE_LOG_SYSTEMS(Verbose, TEXT("[DRIVE][SYS:%s] Initialized"), *GetDebugName());
		return true;
	}

	virtual void Shutdown()
	{
		if (!bInitialized)
		{
			return;
		}

		bInitialized = false;
		Ctx = FDriveSystemContext{};
		DRIVE_LOG_SYSTEMS(Verbose, TEXT("[DRIVE][SYS:%s] Shutdown"), *GetDebugName());
	}

	virtual void TickFixed(float FixedDt) {}
	virtual void PostTickFixed(float FixedDt) {}
	
	virtual void OnRegistered() {}
	virtual void OnUnregistered() {}

protected:
	FDriveSystemContext Ctx;
	bool bInitialized = false;
};
