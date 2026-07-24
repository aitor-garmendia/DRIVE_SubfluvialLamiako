#include "Messaging/DriveMessageBusBridge.h"

#include "Async/Async.h"
#include "UObject/Class.h"

#include "Messaging/FDriveMessageBus.h"
#include "DriveMessageBusTypes.h" 
#include "UObject/UObjectGlobals.h"
#include "Logging/DriveLog.h"

void UDriveMessageBusBridge::Initialize(FDriveMessageBus* InBus)
{
	if (bInitialized)
	{
		return;
	}

	Bus = InBus;
	bInitialized = (Bus != nullptr);
}

void UDriveMessageBusBridge::Shutdown()
{
	if (!bInitialized)
	{
		Bus = nullptr;
		return;
	}

	if (Bus)
	{
		for (auto& KV : SubscriptionHandles)
		{
			const FDriveSubscriptionHandle& Handle = KV.Value;
			Bus->Unsubscribe(Handle);
		}
	}

	SubscriptionHandles.Reset();
	AllowedTypes.Reset();

	Bus = nullptr;
	bInitialized = false;
}

bool UDriveMessageBusBridge::AddEventType(UScriptStruct* EventType)
{
	if (!bInitialized || !Bus || !EventType)
	{
		return false;
	}

	if (AllowedTypes.Contains(EventType))
	{
		return SubscriptionHandles.Contains(EventType);
	}
	AllowedTypes.Add(EventType);

	FDriveSubscriptionHandle Handle = Bus->SubscribeRaw(
		EventType,
		[this, EventType](const void* EventPtr)
		{
			HandleRawEvent(EventType, EventPtr);
		}
	);

	SubscriptionHandles.Add(EventType, Handle);
	return true;
}

bool UDriveMessageBusBridge::AddEventTypeByName(FName StructName)
{
	if (StructName.IsNone())
	{
		return false;
	}

	UScriptStruct* Found = FindObject<UScriptStruct>(nullptr, *StructName.ToString());
	if (!Found)
	{
		DRIVE_LOG_BUS(Log, TEXT("[DRIVE][UE][Bridge] Struct not found: %s"), *StructName.ToString());
		return false;
	}

	return AddEventType(Found);
}

void UDriveMessageBusBridge::ClearEventTypes()
{
	if (!bInitialized || !Bus)
	{
		AllowedTypes.Reset();
		SubscriptionHandles.Reset();
		return;
	}

	for (auto& KV : SubscriptionHandles)
	{
		Bus->Unsubscribe(KV.Value);
	}

	AllowedTypes.Reset();
	SubscriptionHandles.Reset();
}

void UDriveMessageBusBridge::HandleRawEvent(UScriptStruct* EventType, const void* EventPtr)
{
	if (!EventType || !EventPtr)
	{
		return;
	}

	FString EventText;
	EventType->ExportText(EventText, EventPtr, nullptr, nullptr, PPF_None, nullptr);
    DRIVE_LOG_BUS(Log, TEXT("[Bridge SmokeTest] %s -> %s"), *EventType->GetName(), *EventText);

	if (IsInGameThread())
	{
		OnBridgedEvent.Broadcast(EventType->GetFName(), EventText);
	}
	else
	{
		const FName TypeName = EventType->GetFName();
		AsyncTask(ENamedThreads::GameThread, [this, TypeName, EventText]()
		{
			OnBridgedEvent.Broadcast(TypeName, EventText);
		});
	}
}