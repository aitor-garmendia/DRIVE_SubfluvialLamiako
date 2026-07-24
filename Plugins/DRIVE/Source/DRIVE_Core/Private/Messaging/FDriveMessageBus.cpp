#include "Messaging/FDriveMessageBus.h"
#include "Logging/DriveLog.h"

FDriveMessageBus::FDriveMessageBus()
{
	CurrentFrame.FrameIndex = 0;
	LastFrame.FrameIndex = 0;
}

FDriveMessageBus::~FDriveMessageBus()
{
    ClearQueue();
}

void FDriveMessageBus::ResetFrameMetrics()
{
	CurrentFrame.ResetKeepIndex();
}

bool FDriveMessageBus::IsSubscriberAlive(UScriptStruct* EventType, const FGuid& Id) const
{
	const TArray<FSubscriberEntry>* List = SubscribersByEvent.Find(EventType);
	if (!List) return false;

	for (const FSubscriberEntry& E : *List)
	{
		if (E.Id == Id)
		{
			return true;
		}
	}
	return false;
}

FDriveSubscriptionHandle FDriveMessageBus::SubscribeRaw(UScriptStruct* EventType, TFunction<void(const void*)> Callback)
{
    check(EventType);
    check(Callback);

    FDriveSubscriptionHandle Handle(EventType);

    TArray<FSubscriberEntry>& List = SubscribersByEvent.FindOrAdd(EventType);
    List.Add({ Handle.Id, MoveTemp(Callback) }); // orden = orden de alta

    DRIVE_LOG_BUS(Log, TEXT("[DRIVE][BUS] SubscribeRaw: %s"), *EventType->GetName());
    return Handle;
}

bool FDriveMessageBus::Unsubscribe(const FDriveSubscriptionHandle& Handle)
{
    CurrentFrame.UnsubscribeCalls++;
    if (!Handle.IsValid())
    {
        CurrentFrame.Dropped_InvalidArgs++;
        return false;
    }

    TArray<FSubscriberEntry>* List = SubscribersByEvent.Find(Handle.EventType);
    if (!List)
    {
        CurrentFrame.Dropped_InvalidArgs++;
        return false;
    }

    const int32 Before = List->Num();
    List->RemoveAll([&](const FSubscriberEntry& E)
    {
        return E.Id == Handle.Id;
    });

    const int32 Removed = Before - List->Num();
    if (Removed > 0)
	{
		CurrentFrame.UnsubscribeRemoved += (uint32)Removed;
	}

    if (List->Num() == 0)
    {
        SubscribersByEvent.Remove(Handle.EventType);
    }

    DRIVE_LOG_BUS(Warning, TEXT("Unsubscribe: removed=%d"), Removed);
    return Removed > 0;
}

void FDriveMessageBus::Flush()
{
    // “Frame boundary” de instrumentación
	LastFrame = CurrentFrame;
	CurrentFrame.ResetKeepIndex();
	CurrentFrame.FrameIndex = LastFrame.FrameIndex + 1;
	CurrentFrame.QueueSizeAtFlushStart = (uint32)EventQueue.Num();
	CurrentFrame.QueuePeak = FMath::Max(CurrentFrame.QueuePeak, (uint32)EventQueue.Num());

    for (FQueuedEvent& QE : EventQueue)
    {
        if (QE.Type && QE.Data.Num() > 0)
        {
            CurrentFrame.Flushed++;
			CurrentFrame.PublishedByType.FindOrAdd(QE.Type->GetFName())++;
            PublishRaw(QE.Type, QE.Data.GetData());
        } else {
            CurrentFrame.Dropped_InvalidArgs++;
        } 

        QE.Destroy();
    }
    EventQueue.Reset();
}

void FDriveMessageBus::ClearQueue()
{
    for (FQueuedEvent& QE : EventQueue)
    {
        QE.Destroy();
    }
    EventQueue.Reset();
}

void FDriveMessageBus::Clear()
{
    ClearQueue();
    SubscribersByEvent.Reset();
}

void FDriveMessageBus::PublishRaw(UScriptStruct* EventType, const void* EventPtr)
{
    if (!ensureMsgf(EventType != nullptr, TEXT("[DRIVE][BUS] PublishRaw failed: EventType is null")))
	{
		CurrentFrame.Dropped_InvalidArgs++;
		return;
	}
	if (!ensureMsgf(EventPtr != nullptr, TEXT("[DRIVE][BUS] PublishRaw failed: EventPtr is null (%s)"), *EventType->GetName()))
	{
		CurrentFrame.Dropped_InvalidArgs++;
		return;
	}

    TArray<FSubscriberEntry>* List = SubscribersByEvent.Find(EventType);
    if (!List || List->Num() == 0)
    {
        CurrentFrame.Dropped_NoSubscribers++;
        DRIVE_LOG_BUS(Verbose, TEXT("PublishRaw: 0 subscribers"));
        return;
    }

    DRIVE_LOG_BUS(Verbose, TEXT("PublishRaw: %d subscribers"), List->Num());

    // Snapshot para poder modificar subscripciones durante callbacks sin romper iteración
    const TArray<FSubscriberEntry> Snapshot = *List;
    for (const FSubscriberEntry& E : Snapshot)
    {
        if (!IsSubscriberAlive(EventType, E.Id))
		{
			CurrentFrame.Dropped_StaleSubscriber++;
			continue;
		}

        if (E.Fn)
        {
            E.Fn(EventPtr);
            CurrentFrame.Delivered++;
			CurrentFrame.DeliveredByType.FindOrAdd(EventType->GetFName())++;
        }
    }
}