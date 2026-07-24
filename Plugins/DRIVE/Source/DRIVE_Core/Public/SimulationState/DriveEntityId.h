#pragma once

#include "CoreMinimal.h"


struct FDriveEntityId
{
	uint32 Index = 0;
	uint32 Generation = 0;

	FDriveEntityId() = default;
	FDriveEntityId(uint32 InIndex, uint32 InGeneration): Index(InIndex), Generation(InGeneration)
	{
	}

	static FDriveEntityId Invalid()
	{
		return FDriveEntityId(0, 0);
	}

	bool IsValid() const
	{
		return !(Index == 0 && Generation == 0);
	}

	bool operator == (const FDriveEntityId& Other) const
	{
		return Index == Other.Index && Generation == Other.Generation;
	}

	bool operator != (const FDriveEntityId& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FDriveEntityId& Id)
{
	return HashCombine(::GetTypeHash(Id.Index), ::GetTypeHash(Id.Generation));
}