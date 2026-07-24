#pragma once

#include "CoreMinimal.h"
#include "DriveMessageBusTypes.h"
#include "Logging/DriveLog.h"

class DRIVE_CORE_API FDriveMessageBus
{
public:
	FDriveMessageBus();
	~FDriveMessageBus();

	FDriveMessageBus(const FDriveMessageBus&) = delete;
	FDriveMessageBus& operator=(const FDriveMessageBus&) = delete;

	// Metrics
	struct FFrameMetrics
	{
		uint64 FrameIndex = 0;

		// Trafico
		uint32 Enqueued = 0;
		uint32 PublishedImmediate = 0;  
		uint32 Flushed = 0;             
		uint32 Delivered = 0;        

		// Errores
		uint32 Dropped_NoSubscribers = 0;
		uint32 Dropped_StaleSubscriber = 0;
		uint32 Dropped_InvalidArgs = 0;

		// Subscripciones
		uint32 SubscriptionsAdded = 0;
		uint32 UnsubscribeCalls = 0;
		uint32 UnsubscribeRemoved = 0;

		// Cola
		uint32 QueueSizeAtFlushStart = 0;
		uint32 QueuePeak = 0;

		// Breakdown 
		TMap<FName, uint32> PublishedByType;
		TMap<FName, uint32> DeliveredByType;

		void ResetKeepIndex()
		{
			const uint64 KeepIndex = FrameIndex;
			*this = FFrameMetrics{};
			FrameIndex = KeepIndex;
		}
	};

	void ResetFrameMetrics();
	const FFrameMetrics& GetLastFrameMetrics() const { return LastFrame; }
	const FFrameMetrics& GetCurrentFrameMetrics() const { return CurrentFrame; }

	// API
	template <typename EventT>
	FDriveSubscriptionHandle Subscribe(TFunction<void(const EventT&)> Callback)
	{
		UScriptStruct* EventType = EventT::StaticStruct();
		check(EventType);

		if (!ensureMsgf(EventType, TEXT("[DRIVE][BUS] Subscribe failed: EventType is null")))
		{
			CurrentFrame.Dropped_InvalidArgs++;
			return FDriveSubscriptionHandle(); 
		}

		if (!ensureMsgf((bool)Callback, TEXT("[DRIVE][BUS] Subscribe failed: Callback is invalid (%s)"), *EventType->GetName()))
		{
			CurrentFrame.Dropped_InvalidArgs++;
			return FDriveSubscriptionHandle();
		}

		FDriveSubscriptionHandle Handle(EventType);
		FSubscriberFn Wrapper = [Cb = MoveTemp(Callback)](const void* EventPtr)
		{
			check(EventPtr);
			Cb(*reinterpret_cast<const EventT*>(EventPtr));
		};

		TArray<FSubscriberEntry>& List = SubscribersByEvent.FindOrAdd(EventType);
		List.Add({ Handle.Id, MoveTemp(Wrapper) }); 
		CurrentFrame.SubscriptionsAdded++;
		DRIVE_LOG_BUS(Verbose, TEXT("Subscribe: %s"), *EventType->GetName());
		return Handle;
	}

	FDriveSubscriptionHandle SubscribeRaw(UScriptStruct* EventType, TFunction<void(const void* /*EventPtr*/)> Callback);

	bool Unsubscribe(const FDriveSubscriptionHandle& Handle);

	template <typename EventT>
	void Publish(const EventT& Event)
	{
		UScriptStruct* EventType = EventT::StaticStruct();
		check(EventType);

		if (!ensureMsgf(EventType, TEXT("[DRIVE][BUS] Publish failed: EventType is null")))
		{
			CurrentFrame.Dropped_InvalidArgs++;
			return;
		}

		CurrentFrame.PublishedImmediate++;
		CurrentFrame.PublishedByType.FindOrAdd(EventType->GetFName())++;

		PublishRaw(EventType, &Event);
	}

	template <typename EventT>
	void Enqueue(const EventT& Event)
	{
		UScriptStruct* EventType = EventT::StaticStruct();
		check(EventType);

		if (!ensureMsgf(EventType, TEXT("[DRIVE][BUS] Enqueue failed: EventType is null")))
		{
			CurrentFrame.Dropped_InvalidArgs++;
			return;
		}

		FQueuedEvent QE;
		QE.Type = EventType;

		const int32 Size = EventType->GetStructureSize();
		QE.Data.SetNumUninitialized(Size);

		EventType->InitializeStruct(QE.Data.GetData());
		EventType->CopyScriptStruct(QE.Data.GetData(), &Event);
		EventQueue.Add(MoveTemp(QE));

		CurrentFrame.Enqueued++;
		CurrentFrame.QueuePeak = FMath::Max<uint32>(CurrentFrame.QueuePeak, (uint32)EventQueue.Num());
	}

	void Flush();
	void ClearQueue();
	void Clear();

	int32 GetQueuedEventCount() const { return EventQueue.Num(); }
	int32 GetSubscriberEventTypeCount() const { return SubscribersByEvent.Num(); }

	int32 GetTotalSubscriberCount() const
	{
		int32 Total = 0;
		for (const auto& Pair : SubscribersByEvent)
		{
			Total += Pair.Value.Num();
		}
		return Total;
	}

private:
	using FSubscriberFn = TFunction<void(const void* /*EventPtr*/)>;

	struct FQueuedEvent
	{
		UScriptStruct* Type = nullptr;
		TArray<uint8> Data;

		void Destroy()
		{
			if (Type && Data.Num() > 0)
			{
				Type->DestroyStruct(Data.GetData());
			}
			Type = nullptr;
			Data.Reset();
		}
	};

	struct FSubscriberEntry
	{
		FGuid Id;
		FSubscriberFn Fn;
	};

	bool IsSubscriberAlive(UScriptStruct* EventType, const FGuid& Id) const;

	TMap<UScriptStruct*, TArray<FSubscriberEntry>> SubscribersByEvent;
	TArray<FQueuedEvent> EventQueue;
	void PublishRaw(UScriptStruct* EventType, const void* EventPtr);

	FFrameMetrics CurrentFrame{};
	FFrameMetrics LastFrame{};
};