#include "ECS/DriveEntityRegistry.h"

void FDriveEntityRegistry::Reserve(int32 Capacity)
{
	if (Capacity <= 0)
	{
		return;
	}

	Generations.Reserve(Capacity);
	Alive.Reserve(Capacity);
	FreeList.Reserve(Capacity / 4);
}

FDriveEntityId FDriveEntityRegistry::Create()
{
	uint32 Index = 0;

	if (FreeList.Num() > 0)
	{
		Index = FreeList.Pop(EAllowShrinking::No);
		ensureMsgf(Alive.IsValidIndex((int32)Index) && Alive[(int32)Index] == 0, TEXT("[DRIVE][EntityRegistry] Create(): slot %u expected dead but was alive"), Index);

		Alive[(int32)Index] = 1;
		AliveCount++;
		return FDriveEntityId::Make(Index, Generations[(int32)Index]);
	}

	Index = static_cast<uint32>(Generations.Num());
	Generations.Add(0); 
	Alive.Add(1);
	AliveCount++;

	return FDriveEntityId::Make(Index, 0);
}

bool FDriveEntityRegistry::Destroy(FDriveEntityId Entity)
{
	if (!IsAlive(Entity))
	{
		return false;
	}

	const uint32 Index = Entity.GetIndex();
	const int32 I = (int32)Index;

	Alive[I] = 0;
	AliveCount--;
	Generations[I]++;
	FreeList.Add(Index);

	return true;
}

bool FDriveEntityRegistry::IsValid(FDriveEntityId Entity) const
{
	if (!Entity.IsSet())
	{
		return false;
	}

	const uint32 Index = Entity.GetIndex();
	if (!IsIndexInRange(Index))
	{
		return false;
	}

	const int32 I = (int32)Index;
	return Generations[I] == Entity.GetGeneration();
}

bool FDriveEntityRegistry::IsAlive(FDriveEntityId Entity) const
{
	if (!IsValid(Entity))
	{
		return false;
	}

	const int32 I = (int32)Entity.GetIndex();
	return Alive[I] != 0;
}

bool FDriveEntityRegistry::CheckInvariants() const
{
	if (Alive.Num() != Generations.Num())
	{
		return false;
	}

	int32 Count = 0;
	for (uint8 b : Alive)
	{
		Count += (b != 0) ? 1 : 0;
	}
	if (Count != AliveCount)
	{
		return false;
	}

	for (uint32 Idx : FreeList)
	{
		if (Idx < (uint32)Alive.Num())
		{
			if (Alive[(int32)Idx] != 0)
			{
				return false;
			}
		}
	}
	return true;
}