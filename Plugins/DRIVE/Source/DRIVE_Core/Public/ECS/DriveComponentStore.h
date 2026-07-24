#pragma once

#include "CoreMinimal.h"
#include "ECS/DriveEntityId.h"
#include "ECS/DriveEcsTypes.h"

class DRIVE_CORE_API IDriveComponentStore
{
public:
	virtual ~IDriveComponentStore() = default;

	virtual void RemoveForEntity(FDriveEntityId Entity) = 0;
	virtual bool HasForEntity(FDriveEntityId Entity) const = 0;
	virtual int32 Num() const = 0;
};


template<typename TComponent>
class TDriveComponentStore final : public IDriveComponentStore
{
public:
	TDriveComponentStore() = default;
	virtual ~TDriveComponentStore() override = default;

	virtual void RemoveForEntity(FDriveEntityId Entity) override
	{
		Remove(Entity);
	}

	virtual bool HasForEntity(FDriveEntityId Entity) const override
	{
		return Has(Entity);
	}

	virtual int32 Num() const override
	{
		return DenseEntities.Num();
	}

	bool Has(FDriveEntityId Entity) const
	{
		return EntityToIndex.Contains(Entity);
	}

	TComponent& AddOrReplace(FDriveEntityId Entity, const TComponent& Value)
	{
		if (int32* FoundIndex = EntityToIndex.Find(Entity))
		{
			DenseComponents[*FoundIndex] = Value;
			return DenseComponents[*FoundIndex];
		}

		const int32 NewIndex = DenseEntities.Num();
		DenseEntities.Add(Entity);
		DenseComponents.Add(Value);
		EntityToIndex.Add(Entity, NewIndex);

		return DenseComponents[NewIndex];
	}

	template<typename... TArgs>
	TComponent& Emplace(FDriveEntityId Entity, TArgs&&... Args)
	{
		if (int32* FoundIndex = EntityToIndex.Find(Entity))
		{
			DenseComponents[*FoundIndex] = TComponent(Forward<TArgs>(Args)...);
			return DenseComponents[*FoundIndex];
		}

		const int32 NewIndex = DenseEntities.Num();
		DenseEntities.Add(Entity);
		DenseComponents.Emplace(Forward<TArgs>(Args)...);
		EntityToIndex.Add(Entity, NewIndex);

		return DenseComponents[NewIndex];
	}

	bool Remove(FDriveEntityId Entity)
	{
		int32 IndexToRemove = INDEX_NONE;
		if (!EntityToIndex.RemoveAndCopyValue(Entity, IndexToRemove))
		{
			return false;
		}

		const int32 LastIndex = DenseEntities.Num() - 1;

		if (IndexToRemove != LastIndex)
		{
			const FDriveEntityId MovedEntity = DenseEntities[LastIndex];
			DenseEntities[IndexToRemove]   = DenseEntities[LastIndex];
			DenseComponents[IndexToRemove] = MoveTemp(DenseComponents[LastIndex]);
			EntityToIndex.Add(MovedEntity, IndexToRemove);
		}

		DenseEntities.Pop(EAllowShrinking::No);
        DenseComponents.Pop(EAllowShrinking::No);

		return true;
	}

	TComponent* Get(FDriveEntityId Entity)
	{
		if (int32* FoundIndex = EntityToIndex.Find(Entity))
		{
			return &DenseComponents[*FoundIndex];
		}
		return nullptr;
	}

	const TComponent* Get(FDriveEntityId Entity) const
	{
		if (const int32* FoundIndex = EntityToIndex.Find(Entity))
		{
			return &DenseComponents[*FoundIndex];
		}
		return nullptr;
	}

	TComponent& GetChecked(FDriveEntityId Entity)
	{
		TComponent* Ptr = Get(Entity);
		checkf(Ptr != nullptr, TEXT("Component not found for Entity=%s"), *Entity.ToString());
		return *Ptr;
	}

	const TComponent& GetChecked(FDriveEntityId Entity) const
	{
		const TComponent* Ptr = Get(Entity);
		checkf(Ptr != nullptr, TEXT("Component not found for Entity=%s"), *Entity.ToString());
		return *Ptr;
	}

	void Reset()
	{
		DenseEntities.Reset();
		DenseComponents.Reset();
		EntityToIndex.Reset();
	}

private:
	TArray<FDriveEntityId> DenseEntities;
	TArray<TComponent>     DenseComponents;
	TMap<FDriveEntityId, int32> EntityToIndex;
};