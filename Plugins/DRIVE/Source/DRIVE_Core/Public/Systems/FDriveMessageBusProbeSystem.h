#pragma once

#include "CoreMinimal.h"
#include "Systems/DriveSystemBase.h"
#include "Messaging/FDriveMessageBus.h"

// System de validación manual
class DRIVE_CORE_API FDriveMessageBusProbeSystem final : public FDriveSystemBase
{
public:
	FDriveMessageBusProbeSystem() = default;

	// Identidad 
	virtual FName GetSystemId() const override { return TEXT("Drive.MessageBusProbe"); }
	virtual FString GetDebugName() const override { return TEXT("FDriveMessageBusProbeSystem"); }

	virtual int32 GetInitOrder() const override { return 0; }
	virtual int32 GetFixedTickOrder() const override { return 0; }

	// Ciclo de vida
	virtual bool Initialize(const FDriveSystemContext& Context) override;
	virtual void Shutdown() override;

	// Tick 
	virtual void TickFixed(float FixedDt) override;
	virtual void PostTickFixed(float FixedDt) override {}

private:
	FDriveMessageBus* Bus = nullptr;

	bool bDidPublishWithoutSubscribers = false;

	FDriveSubscriptionHandle HandleA;
	FDriveSubscriptionHandle HandleB;

	int32 StepCounter = 0;

	void LogMetrics(const TCHAR* Tag) const;
};
