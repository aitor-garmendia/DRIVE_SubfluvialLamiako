#pragma once

#include "CoreMinimal.h"
#include "Messaging/FDriveMessageBus.h"
#include "Messaging/DriveBusTestEvents.h"
#include "Logging/DriveLog.h"

class FDriveBusProbeSystem
{
public:
	explicit FDriveBusProbeSystem(FDriveMessageBus& InBus): Bus(InBus)
	{}

	void Initialize()
	{
		// Subscriber A: al recibir, se desuscribe
		HandleA = Bus.Subscribe<FDriveBusTestEvent>([this](const FDriveBusTestEvent& E)
		{
			DRIVE_LOG_BUS(Log, TEXT("[BusProbe] A received Value=%d, now Unsubscribe(A)"), E.Value);
			Bus.Unsubscribe(HandleA);
		});

		// Subscriber B: normal
		HandleB = Bus.Subscribe<FDriveBusTestEvent>([](const FDriveBusTestEvent& E)
		{
			DRIVE_LOG_BUS(Log, TEXT("[BusProbe] B received Value=%d"), E.Value);
		});

		bInitDone = true;
	}

	void Tick_Probe()
	{
		if (!bInitDone)
		{
			Initialize();
		}

		// 1) Publish sin subs
		// 2) Publish inmediato
		{
			FDriveBusTestEvent Ev;
			Ev.Value = StepCounter * 10 + 1;
			Bus.Publish(Ev);
		}

		// 3) Enqueue 
		{
			FDriveBusTestEvent Ev;
			Ev.Value = StepCounter * 10 + 2;
			Bus.Enqueue(Ev);
		}
		StepCounter++;

		// Log de métricas
		const auto& M = Bus.GetCurrentFrameMetrics();
		DRIVE_LOG_BUS(Log,
			TEXT("[BusProbe] Frame=%llu Enq=%u PubImm=%u Flushed=%u Delivered=%u DropNoSubs=%u DropStale=%u DropInvalid=%u SubAdded=%u UnsubCalls=%u UnsubRemoved=%u QStart=%u QPeak=%u"),
			M.FrameIndex, M.Enqueued, M.PublishedImmediate, M.Flushed, M.Delivered, M.Dropped_NoSubscribers, M.Dropped_StaleSubscriber, M.Dropped_InvalidArgs, 
			M.SubscriptionsAdded, M.UnsubscribeCalls, M.UnsubscribeRemoved, M.QueueSizeAtFlushStart, M.QueuePeak);
	}

	void Probe_PublishWithoutSubscribers()
	{
		FDriveBusTestEvent Ev;
		Ev.Value = 999;
		Bus.Publish(Ev);

		const auto& M = Bus.GetCurrentFrameMetrics();
		DRIVE_LOG_BUS(Log, TEXT("[BusProbe] After Publish(no subs): DropNoSubs=%u PubImm=%u Delivered=%u"),
			M.Dropped_NoSubscribers, M.PublishedImmediate, M.Delivered);
	}

private:
	FDriveMessageBus& Bus;

	bool bInitDone = false;
	int32 StepCounter = 0;

	FDriveSubscriptionHandle HandleA;
	FDriveSubscriptionHandle HandleB;
};