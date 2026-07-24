#pragma once

#include "CoreMinimal.h"

using FDriveComponentTypeId = uint32;

struct IDriveComponent
{
};


template <typename TComponent>
struct TDriveComponentInfo
{
	static FName Name()
	{
		return FName(TComponent::StaticComponentName());
	}

	static FDriveComponentTypeId TypeId()
	{
		return ::GetTypeHash(Name());
	}
};

#define DRIVE_DECLARE_COMPONENT(ComponentTypeName) \
	static const TCHAR* StaticComponentName() { return TEXT(#ComponentTypeName); }