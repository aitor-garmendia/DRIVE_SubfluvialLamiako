#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"

struct FDriveEntity
{
	int32 Id = INDEX_NONE;
	int32 Generation = 0;

	FDriveEntity() = default;
	FDriveEntity(int32 InId, int32 InGen) : Id(InId), Generation(InGen) {}

	bool IsValid() const { return Id != INDEX_NONE; }

	friend bool operator==(const FDriveEntity& A, const FDriveEntity& B)
	{
		return A.Id == B.Id && A.Generation == B.Generation;
	}
	friend bool operator!=(const FDriveEntity& A, const FDriveEntity& B)
	{
		return !(A == B);
	}
};

FORCEINLINE uint32 GetTypeHash(const FDriveEntity& E)
{
	return HashCombine(::GetTypeHash(E.Id), ::GetTypeHash(E.Generation));
}

class DRIVE_CORE_API IDriveComponentStorage
{
public:
	virtual ~IDriveComponentStorage() = default;

	virtual void RemoveForEntity(int32 EntityId) = 0;
	virtual bool Has(int32 EntityId) const = 0;
	virtual int32 Num() const = 0;
};

template <typename TComponent>
class TDriveComponentStorage final : public IDriveComponentStorage
{
public:
	virtual void RemoveForEntity(int32 EntityId) override
	{
		const int32* DenseIndexPtr = Sparse.Find(EntityId);
		if (!DenseIndexPtr)
		{
			return;
		}

		const int32 DenseIndex = *DenseIndexPtr;
		const int32 LastIndex = DenseEntities.Num() - 1;

		if (DenseIndex != LastIndex)
		{
			DenseEntities[DenseIndex] = DenseEntities[LastIndex];
			DenseComponents[DenseIndex] = MoveTemp(DenseComponents[LastIndex]);
			Sparse[DenseEntities[DenseIndex]] = DenseIndex;
		}

		DenseEntities.RemoveAt(LastIndex);
		DenseComponents.RemoveAt(LastIndex);
		Sparse.Remove(EntityId);
	}

	virtual bool Has(int32 EntityId) const override
	{
		return Sparse.Contains(EntityId);
	}

	virtual int32 Num() const override
	{
		return DenseEntities.Num();
	}

	TComponent& AddOrGet(int32 EntityId)
	{
		if (int32* DenseIndexPtr = Sparse.Find(EntityId))
		{
			return DenseComponents[*DenseIndexPtr];
		}

		const int32 NewDenseIndex = DenseEntities.Num();
		DenseEntities.Add(EntityId);
		DenseComponents.Add(TComponent{});
		Sparse.Add(EntityId, NewDenseIndex);
		return DenseComponents[NewDenseIndex];
	}

	template <typename... TArgs>
	TComponent& Emplace(int32 EntityId, TArgs&&... Args)
	{
		if (int32* DenseIndexPtr = Sparse.Find(EntityId))
		{
			DenseComponents[*DenseIndexPtr] = TComponent(Forward<TArgs>(Args)...);
			return DenseComponents[*DenseIndexPtr];
		}

		const int32 NewDenseIndex = DenseEntities.Num();
		DenseEntities.Add(EntityId);
		DenseComponents.Emplace(Forward<TArgs>(Args)...);
		Sparse.Add(EntityId, NewDenseIndex);
		return DenseComponents[NewDenseIndex];
	}

	TComponent* Get(int32 EntityId)
	{
		if (int32* DenseIndexPtr = Sparse.Find(EntityId))
		{
			return &DenseComponents[*DenseIndexPtr];
		}
		return nullptr;
	}

	const TComponent* Get(int32 EntityId) const
	{
		if (const int32* DenseIndexPtr = Sparse.Find(EntityId))
		{
			return &DenseComponents[*DenseIndexPtr];
		}
		return nullptr;
	}

private:
	TArray<int32> DenseEntities;
	TArray<TComponent> DenseComponents;
	TMap<int32, int32> Sparse;
};

class DRIVE_CORE_API FDriveSimState
{
public:
	FDriveSimState() = default;
    ~FDriveSimState() = default;

	FDriveSimState(const FDriveSimState&) = delete;
	FDriveSimState& operator=(const FDriveSimState&) = delete;

	FDriveSimState(FDriveSimState&&) = default;
	FDriveSimState& operator=(FDriveSimState&&) = default;


	FDriveEntity CreateEntity();
	void DestroyEntity(FDriveEntity Entity);
	bool IsAlive(FDriveEntity Entity) const;

	int32 GetAliveCount() const { return AliveCount; }

	int32 GetStorageTypeCount() const
	{
		return Storages.Num();
	}

	int32 GetTotalComponentCount() const
	{
		int32 Total = 0;
		for (const auto& Pair : Storages)
		{
			if (Pair.Value.IsValid())
			{
				Total += Pair.Value->Num();
			}
		}
		return Total;
	}
	
    template <typename TComponent>
    bool HasComponent(FDriveEntity Entity) const
    {
        if (!IsAlive(Entity)) return false;

        const TSharedPtr<IDriveComponentStorage>* Found = Storages.Find(GetComponentTypeKey<TComponent>());
        if (!Found || !Found->IsValid()) return false;

        return (*Found)->Has(Entity.Id);
    }


	template <typename TComponent, typename... TArgs>
	TComponent& AddComponent(FDriveEntity Entity, TArgs&&... Args)
	{
		check(IsAlive(Entity));

		TDriveComponentStorage<TComponent>* Storage = GetOrCreateStorage<TComponent>();
		return Storage->Emplace(Entity.Id, Forward<TArgs>(Args)...);
	}

	template <typename TComponent>
	void RemoveComponent(FDriveEntity Entity)
	{
		if (!IsAlive(Entity)) return;
		if (TDriveComponentStorage<TComponent>* Storage = GetStorage<TComponent>())
		{
			Storage->RemoveForEntity(Entity.Id);
		}
	}

	template <typename TComponent>
	TComponent* GetComponent(FDriveEntity Entity)
	{
		if (!IsAlive(Entity)) return nullptr;
		if (TDriveComponentStorage<TComponent>* Storage = GetStorage<TComponent>())
		{
			return Storage->Get(Entity.Id);
		}
		return nullptr;
	}

	template <typename TComponent>
	const TComponent* GetComponent(FDriveEntity Entity) const
	{
		if (!IsAlive(Entity)) return nullptr;
		const TDriveComponentStorage<TComponent>* Storage = GetStorage<TComponent>();
		return Storage ? Storage->Get(Entity.Id) : nullptr;
	}

	void Reset();

private:
	TArray<int32> Generations;
	TArray<int32> FreeList;
	int32 AliveCount = 0;

    TMap<FName, TSharedPtr<IDriveComponentStorage>> Storages;

	template <typename TComponent>
	static FName GetComponentTypeKey()
	{
		return FName(TNameOf<TComponent>::GetName());
	}

    template <typename TComponent>
    TDriveComponentStorage<TComponent>* GetStorage()
    {
        TSharedPtr<IDriveComponentStorage>* Found = Storages.Find(GetComponentTypeKey<TComponent>());
        if (!Found || !Found->IsValid()) return nullptr;

        return static_cast<TDriveComponentStorage<TComponent>*>(Found->Get());
    }

    template <typename TComponent>
    const TDriveComponentStorage<TComponent>* GetStorage() const
    {
        const TSharedPtr<IDriveComponentStorage>* Found = Storages.Find(GetComponentTypeKey<TComponent>());
        if (!Found || !Found->IsValid()) return nullptr;

        return static_cast<const TDriveComponentStorage<TComponent>*>(Found->Get());
    }

    template <typename TComponent>
    TDriveComponentStorage<TComponent>* GetOrCreateStorage()
    {
        const FName Key = GetComponentTypeKey<TComponent>();

        if (!Storages.Contains(Key))
        {
            Storages.Add(Key, MakeShared<TDriveComponentStorage<TComponent>>());
        }

        return GetStorage<TComponent>();
    }

	void RemoveAllComponentsForEntity(int32 EntityId);
};