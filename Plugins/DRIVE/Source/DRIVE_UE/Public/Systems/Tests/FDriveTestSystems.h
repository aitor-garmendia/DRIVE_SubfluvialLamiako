#pragma once

#include "CoreMinimal.h"
#include "Systems/DriveSystemBase.h"
#include "Messaging/FDriveMessageBus.h"
#include "Simulation/DriveTestEvents.h"
#include "Logging/DriveLog.h"

class FDriveSystem_TestPingResponder final : public FDriveSystemBase
{
public:
	FDriveSystem_TestPingResponder() = default;
	virtual ~FDriveSystem_TestPingResponder() override = default;

	virtual FName GetSystemId() const override { return TEXT("TestPingResponder"); }

	virtual bool Initialize(const FDriveSystemContext& Context) override
	{
		if (!FDriveSystemBase::Initialize(Context))
		{
			return false;
		}

		check(Ctx.Bus);

		PingHandle = Ctx.Bus->Subscribe<FDriveEvent_TestPing>(
			[this](const FDriveEvent_TestPing& E)
			{
				DRIVE_LOG_SYSTEMS(Warning, TEXT("[DRIVE][SYS:Responder] Received PING Value=%d -> enqueue PONG"), E.Value);

				FDriveEvent_TestPong Pong;
				Pong.Value = E.Value + 1;
				Ctx.Bus->Enqueue(Pong);
			}
		);
		DRIVE_LOG_SYSTEMS(Warning, TEXT("[DRIVE][SYS:Responder] Initialized"));
		return true;
	}

	virtual void Shutdown() override
	{
		if (Ctx.Bus && PingHandle.IsValid())
		{
			Ctx.Bus->Unsubscribe(PingHandle);
			PingHandle.Reset();
		}

		FDriveSystemBase::Shutdown();
		DRIVE_LOG_SYSTEMS(Log, TEXT("[DRIVE][SYS:Responder] Shutdown"));
	}

private:
	FDriveSubscriptionHandle PingHandle;
};

class FDriveSystem_TestPingPublisher final : public FDriveSystemBase
{
public:
	FDriveSystem_TestPingPublisher() = default;
	virtual ~FDriveSystem_TestPingPublisher() override = default;

	virtual FName GetSystemId() const override { return TEXT("TestPingPublisher"); }

	virtual bool Initialize(const FDriveSystemContext& Context) override
	{
		if (!FDriveSystemBase::Initialize(Context))
		{
			return false;
		}

		check(Ctx.Bus);

		PongHandle = Ctx.Bus->Subscribe<FDriveEvent_TestPong>(
			[this](const FDriveEvent_TestPong& E)
			{
				DRIVE_LOG_SYSTEMS(Warning, TEXT("[DRIVE][SYS:Publisher] Received PONG Value=%d"), E.Value);
			}
		);
		DRIVE_LOG_SYSTEMS(Warning, TEXT("[DRIVE][SYS:Publisher] Initialized"));
		return true;
	}

	virtual void Shutdown() override
	{
		if (Ctx.Bus && PongHandle.IsValid())
		{
			Ctx.Bus->Unsubscribe(PongHandle);
			PongHandle.Reset();
		}

		FDriveSystemBase::Shutdown();
		DRIVE_LOG_SYSTEMS(Log, TEXT("[DRIVE][SYS:Publisher] Shutdown"));
	}

	virtual void TickFixed(float FixedDt) override
	{
		StepCounter++;
		if (StepCounter % 30 != 0) 
		{
			return;
		}

		FDriveEvent_TestPing Ping;
		Ping.Value = StepCounter;

		DRIVE_LOG_SYSTEMS(Warning, TEXT("[DRIVE][SYS:Publisher] Enqueue PING Value=%d (dt=%.3f)"), Ping.Value, FixedDt);
		Ctx.Bus->Enqueue(Ping);
	}

private:
	int32 StepCounter = 0;
	FDriveSubscriptionHandle PongHandle;
};