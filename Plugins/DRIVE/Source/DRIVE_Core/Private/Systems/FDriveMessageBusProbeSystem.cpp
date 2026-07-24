#include "Systems/FDriveMessageBusProbeSystem.h"
#include "Systems/DriveSystemContext.h"
#include "Messaging/DriveBusTestEvents.h"
#include "Logging/DriveLog.h"

bool FDriveMessageBusProbeSystem::Initialize(const FDriveSystemContext& Context)
{
    const bool bBaseOk = FDriveSystemBase::Initialize(Context);
    if (!bBaseOk)
    {
        return false;
    }

	Bus = Context.Bus;
	if (!Bus)
	{
		DRIVE_LOG_SYSTEMS(Error, TEXT("[BusProbe] Initialize failed: Bus is null"));
		return false;
	}

	// Publish SIN suscriptores
	if (!bDidPublishWithoutSubscribers)
	{
		FDriveBusTestEvent Ev;
		Ev.Value = 999;
		Bus->Publish(Ev);

		LogMetrics(TEXT("After Publish(no subs)"));
		bDidPublishWithoutSubscribers = true;
	}

	// Subscriber A: se auto-desuscribe
	HandleA = Bus->Subscribe<FDriveBusTestEvent>([this](const FDriveBusTestEvent& E)
	{
		DRIVE_LOG_SYSTEMS(Log, TEXT("[BusProbe] A received Value=%d -> Unsubscribe(A)"), E.Value);
		if (Bus)
		{
			Bus->Unsubscribe(HandleA);
		}
	});

	// Subscriber B: normal
	HandleB = Bus->Subscribe<FDriveBusTestEvent>([](const FDriveBusTestEvent& E)
	{
		DRIVE_LOG_SYSTEMS(Log, TEXT("[BusProbe] B received Value=%d"), E.Value);
	});

	LogMetrics(TEXT("After Subscribe(A,B)"));
	return true;
}

void FDriveMessageBusProbeSystem::Shutdown()
{
	if (Bus)
	{
		Bus->Unsubscribe(HandleA);
		Bus->Unsubscribe(HandleB);
	}
	Bus = nullptr;
    FDriveSystemBase::Shutdown();
}

void FDriveMessageBusProbeSystem::TickFixed(float FixedDt)
{
	if (!Bus) return;

	// Enqueue 
	{
		FDriveBusTestEvent Ev;
		Ev.Value = StepCounter * 10 + 2;
		Bus->Enqueue(Ev);
	}

	// Publish inmediato 
	{
		FDriveBusTestEvent Ev;
		Ev.Value = StepCounter * 10 + 1;
		Bus->Publish(Ev);
	}

	LogMetrics(TEXT("TickFixed"));
	StepCounter++;
}

void FDriveMessageBusProbeSystem::LogMetrics(const TCHAR* Tag) const
{
	if (!Bus) return;

	const auto& M = Bus->GetCurrentFrameMetrics();

	DRIVE_LOG_SYSTEMS(Log,
		TEXT("[BusProbe][%s] Frame=%llu Enq=%u PubImm=%u Flushed=%u Delivered=%u DropNoSubs=%u DropStale=%u DropInvalid=%u SubAdded=%u UnsubCalls=%u UnsubRemoved=%u QStart=%u QPeak=%u"),
		Tag, (unsigned long long)M.FrameIndex, M.Enqueued, M.PublishedImmediate, M.Flushed, M.Delivered, M.Dropped_NoSubscribers, M.Dropped_StaleSubscriber,
		M.Dropped_InvalidArgs, M.SubscriptionsAdded, M.UnsubscribeCalls, M.UnsubscribeRemoved, M.QueueSizeAtFlushStart, M.QueuePeak);
}
