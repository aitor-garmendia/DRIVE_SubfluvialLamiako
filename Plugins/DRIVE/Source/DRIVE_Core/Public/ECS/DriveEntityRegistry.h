#pragma once

#include "CoreMinimal.h"
#include "ECS/DriveEntityId.h"

class DRIVE_CORE_API FDriveEntityRegistry
{
public:
	FDriveEntityRegistry() = default;

	void Reserve(int32 Capacity);
	FDriveEntityId Create();
	bool Destroy(FDriveEntityId Entity);
	bool IsAlive(FDriveEntityId Entity) const;
	bool IsValid(FDriveEntityId Entity) const;
	int32 GetCapacity() const { return Generations.Num(); }
	int32 GetAliveCount() const { return AliveCount; }
	bool CheckInvariants() const;

private:
	bool IsIndexInRange(uint32 Index) const
	{
		return Index < static_cast<uint32>(Generations.Num());
	}

private:
	TArray<uint32> Generations;
	TArray<uint8> Alive; 
	TArray<uint32> FreeList;
	int32 AliveCount = 0;
};