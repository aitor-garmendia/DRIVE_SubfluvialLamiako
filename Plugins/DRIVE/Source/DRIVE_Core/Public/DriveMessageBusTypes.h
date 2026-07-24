#pragma once

#include "CoreMinimal.h"

struct DRIVE_CORE_API FDriveSubscriptionHandle
{
	FGuid Id;
	UScriptStruct* EventType = nullptr;

	FDriveSubscriptionHandle(): Id(FGuid::NewGuid()), EventType(nullptr)
	{
	}

	explicit FDriveSubscriptionHandle(UScriptStruct* InEventType): Id(FGuid::NewGuid()), EventType(InEventType)
	{
	}

	bool IsValid() const
	{
		return EventType != nullptr && Id.IsValid();
	}

	void Reset()
	{
		Id.Invalidate();
		EventType = nullptr;
	}
};