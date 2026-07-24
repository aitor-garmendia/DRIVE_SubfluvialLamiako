#pragma once

#include "CoreMinimal.h"
#include "ECS/DriveComponentStore.h"
#include "ECS/DriveEcsTypes.h"

class DRIVE_CORE_API FDriveComponentStorage
{
public:
	FDriveComponentStorage() = default;
	~FDriveComponentStorage() = default;

	FDriveComponentStorage(const FDriveComponentStorage&) = delete;
	FDriveComponentStorage& operator=(const FDriveComponentStorage&) = delete;

	template<typename TComponent>
	TDriveComponentStore<TComponent>& GetOrCreateStore()
	{
		const void* TypeKey = MakeTypeKey<TComponent>();

		if (TUniquePtr<IDriveComponentStore>* Found = Stores.Find(TypeKey))
		{
			return *static_cast<TDriveComponentStore<TComponent>*>(Found->Get());
		}

		TUniquePtr<IDriveComponentStore> NewStore = MakeUnique<TDriveComponentStore<TComponent>>();
		TDriveComponentStore<TComponent>* Raw = static_cast<TDriveComponentStore<TComponent>*>(NewStore.Get());

		Stores.Add(TypeKey, MoveTemp(NewStore));
		return *Raw;
	}

	template<typename TComponent>
	TDriveComponentStore<TComponent>* FindStore()
	{
		const void* TypeKey = MakeTypeKey<TComponent>();

		if (TUniquePtr<IDriveComponentStore>* Found = Stores.Find(TypeKey))
		{
			return static_cast<TDriveComponentStore<TComponent>*>(Found->Get());
		}
		return nullptr;
	}

	template<typename TComponent>
	const TDriveComponentStore<TComponent>* FindStore() const
	{
		const void* TypeKey = MakeTypeKey<TComponent>();

		if (const TUniquePtr<IDriveComponentStore>* Found = Stores.Find(TypeKey))
		{
			return static_cast<const TDriveComponentStore<TComponent>*>(Found->Get());
		}
		return nullptr;
	}

	void RemoveAllComponents(const FDriveEntityId Entity)
	{
		for (TPair<const void*, TUniquePtr<IDriveComponentStore>>& Kvp : Stores)
		{
			Kvp.Value->RemoveForEntity(Entity);
		}
	}

	int32 NumStores() const
	{
		return Stores.Num();
	}

	void Reset()
	{
		Stores.Reset();
	}

private:
	template<typename TComponent>
	static const void* MakeTypeKey()
	{
		static uint8 Unique;
		return &Unique;
	}

private:
	TMap<const void*, TUniquePtr<IDriveComponentStore>> Stores;
};
