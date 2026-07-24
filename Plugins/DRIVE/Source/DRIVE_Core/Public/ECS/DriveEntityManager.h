#pragma once

#include "CoreMinimal.h"
#include "ECS/DriveEcsTypes.h" 

class DRIVE_CORE_API FDriveEntityManager
{
public:
	FDriveEntityManager() = default;

	FDriveEntity Create()
	{
		int32 NewId = 0;

		if (FreeList.Num() > 0)
		{
			NewId = FreeList.Pop(EAllowShrinking::No); 
		}
		else
		{
			NewId = ++LastId;
			Generations.SetNum(LastId + 1);
			Alive.SetNum(LastId + 1);
		}

		Alive[NewId] = true;
		return FDriveEntity{ NewId, Generations[NewId] };
	}

	bool IsAlive(const FDriveEntity& E) const
	{
		if (!E.IsValid() || E.Id >= Alive.Num())
		{
			return false;
		}
		return Alive[E.Id] && Generations[E.Id] == E.Generation;
	}

	void Destroy(const FDriveEntity& E)
	{
		if (!IsAlive(E))
		{
			return;
		}

		Alive[E.Id] = false;
		Generations[E.Id]++; 
		FreeList.Add(E.Id);
	}

	int32 GetAliveCount() const
	{
		int32 Count = 0;
		for (int32 i = 1; i < Alive.Num(); ++i)
		{
			if (Alive[i]) { ++Count; }
		}
		return Count;
	}

private:
	int32 LastId = 0;
	TArray<int32> Generations;
	TArray<bool> Alive;
	TArray<int32> FreeList;     
};
