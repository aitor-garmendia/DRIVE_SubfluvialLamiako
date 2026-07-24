#pragma once

#include "CoreMinimal.h"

struct DRIVE_CORE_API FDriveEntityId
{
public:
	static constexpr uint32 InvalidIndex = TNumericLimits<uint32>::Max();

	FDriveEntityId() = default;

	static FDriveEntityId Make(uint32 InIndex, uint32 InGeneration)
	{
		FDriveEntityId Id;
		Id.Index = InIndex;
		Id.Generation = InGeneration;
		return Id;
	}

    static FDriveEntityId Invalid()
	{
		return FDriveEntityId(); 
	}

	bool IsSet() const
	{
		return Index != InvalidIndex;
	}

	bool IsValid() const
	{
		return IsSet();
	}

	void Reset()
	{
		Index = InvalidIndex;
		Generation = 0;
	}

	//uint32 GetIndex() const { return Index; }
	//uint32 GetGeneration() const { return Generation; }
    FORCEINLINE uint32 GetIndex() const { return Index; }
    FORCEINLINE uint32 GetGeneration() const { return Generation; }

	FString ToString() const
	{
		return FString::Printf(TEXT("Entity(Index=%u, Gen=%u)"), Index, Generation);
	}

	bool operator==(const FDriveEntityId& Other) const
	{
		return Index == Other.Index && Generation == Other.Generation;
	}

	bool operator!=(const FDriveEntityId& Other) const
	{
		return !(*this == Other);
	}

private:
	uint32 Index = InvalidIndex;
	uint32 Generation = 0;
};

FORCEINLINE uint32 GetTypeHash(const FDriveEntityId& Id)
{
	return HashCombine(::GetTypeHash(Id.GetIndex()), ::GetTypeHash(Id.GetGeneration()));
}

FORCEINLINE FString LexToString(const FDriveEntityId& Id)
{
	return Id.ToString();
}
