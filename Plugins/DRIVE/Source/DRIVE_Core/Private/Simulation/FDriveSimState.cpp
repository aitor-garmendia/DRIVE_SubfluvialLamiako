#include "Simulation/FDriveSimState.h"

FDriveEntity FDriveSimState::CreateEntity()
{
	int32 NewId = INDEX_NONE;

	if (FreeList.Num() > 0)
	{
		NewId = FreeList.Pop(EAllowShrinking::No);
	}
	else
	{
		NewId = Generations.Num();
		Generations.Add(0);
	}

	AliveCount++;
	return FDriveEntity(NewId, Generations[NewId]);
}

void FDriveSimState::DestroyEntity(FDriveEntity Entity)
{
	if (!IsAlive(Entity))
	{
		return;
	}

	RemoveAllComponentsForEntity(Entity.Id);

	Generations[Entity.Id] += 1;
	FreeList.Add(Entity.Id);
	AliveCount = FMath::Max(0, AliveCount - 1);
}

bool FDriveSimState::IsAlive(FDriveEntity Entity) const
{
	if (!Entity.IsValid()) return false;
	if (!Generations.IsValidIndex(Entity.Id)) return false;
	return Generations[Entity.Id] == Entity.Generation;
}

void FDriveSimState::RemoveAllComponentsForEntity(int32 EntityId)
{
    for (TPair<FName, TSharedPtr<IDriveComponentStorage>>& Pair : Storages)
    {
        if (Pair.Value.IsValid())
        {
            Pair.Value->RemoveForEntity(EntityId);
        }
    }
}

void FDriveSimState::Reset()
{
	Storages.Reset();
	Generations.Reset();
	FreeList.Reset();
	AliveCount = 0;
}
