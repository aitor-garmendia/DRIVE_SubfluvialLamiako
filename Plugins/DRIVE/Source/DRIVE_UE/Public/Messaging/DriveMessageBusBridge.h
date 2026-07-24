#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DriveMessageBusTypes.h"

#include "DriveMessageBusBridge.generated.h"

class FDriveMessageBus;
//struct FDriveSubscriptionHandle;

UCLASS(BlueprintType)
class DRIVE_UE_API UDriveMessageBusBridge : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriveBridgedEventBP, FName, EventType, FString, EventText);
	UPROPERTY(BlueprintAssignable, Category="DRIVE|Messaging")
	FOnDriveBridgedEventBP OnBridgedEvent;

public:
	void Initialize(FDriveMessageBus* InBus);
	void Shutdown();

	bool AddEventType(UScriptStruct* EventType);
	bool AddEventTypeByName(FName StructName);
	void ClearEventTypes();

private:
	void HandleRawEvent(UScriptStruct* EventType, const void* EventPtr);

private:
	FDriveMessageBus* Bus = nullptr;
	TSet<TObjectPtr<UScriptStruct>> AllowedTypes;
	TMap<TObjectPtr<UScriptStruct>, FDriveSubscriptionHandle> SubscriptionHandles;

	bool bInitialized = false;
};