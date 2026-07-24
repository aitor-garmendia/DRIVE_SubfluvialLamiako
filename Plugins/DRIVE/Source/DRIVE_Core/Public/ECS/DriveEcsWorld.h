#pragma once

#include "CoreMinimal.h"
#include "ECS/DriveEntityManager.h"
#include "ECS/DriveComponentStorage.h"
#include "ECS/DriveEcsCommandBuffer.h"
#include "ECS/DriveEcsTypes.h"
#include "Templates/SharedPointer.h"


class DRIVE_CORE_API FDriveEcsWorld
{
public:
	FDriveEcsWorld() = default;

	void BeginFixedTick()
	{
		bInFixedTick = true;
		FlushDeferred();
	}

	void EndFixedTick()
	{
		FlushDeferred();
		bInFixedTick = false;
	}

	bool IsInFixedTick() const { return bInFixedTick; }

	int32 FlushDeferred()
	{
		if (!CmdBuffer.HasPending())
		{
			return 0;
		}
		return CmdBuffer.Flush(*this);
	}

	FDriveEntity CreateEntity()
	{
		if (bInFixedTick)
		{
			FDriveEntity Placeholder{ 0,0 }; 
			CmdBuffer.Enqueue([](FDriveEcsWorld& W)
			{
				W.EntityMgr.Create();
			});
			return Placeholder;
		}

		return EntityMgr.Create();
	}

	void DestroyEntity(const FDriveEntity& E)
	{
		if (bInFixedTick)
		{
			CmdBuffer.Enqueue([E](FDriveEcsWorld& W)
			{
				W.DestroyEntity_Immediate(E);
			});
			return;
		}

		DestroyEntity_Immediate(E);
	}

	bool IsAlive(const FDriveEntity& E) const
	{
		return EntityMgr.IsAlive(E);
	}

	int32 GetAliveCount() const
	{
		return EntityMgr.GetAliveCount();
	}

	template<typename TComponent>
	bool HasComponent(const FDriveEntity& E)
	{
		if (!IsAlive(E)) return false;
		return GetOrCreateStorage<TComponent>().Has(E);
	}

	template<typename TComponent>
	const TComponent* GetComponent(const FDriveEntity& E) const
	{
		if (!IsAlive(E)) return nullptr;
		const TDriveComponentStorage<TComponent>* Storage = FindStorage<TComponent>();
		return Storage ? Storage->Get(E) : nullptr;
	}

	template<typename TComponent>
	TComponent* GetComponentMutable(const FDriveEntity& E)
	{
		if (!IsAlive(E)) return nullptr;
		TDriveComponentStorage<TComponent>* Storage = FindStorageMutable<TComponent>();
		return Storage ? Storage->GetMutable(E) : nullptr;
	}

	template<typename TComponent>
	void AddOrSetComponent(const FDriveEntity& E, const TComponent& Value)
	{
		if (bInFixedTick)
		{
			CmdBuffer.Enqueue([E, Value](FDriveEcsWorld& W)
			{
				W.AddOrSetComponent_Immediate<TComponent>(E, Value);
			});
			return;
		}

		AddOrSetComponent_Immediate<TComponent>(E, Value);
	}

	template<typename TComponent>
	void RemoveComponent(const FDriveEntity& E)
	{
		if (bInFixedTick)
		{
			CmdBuffer.Enqueue([E](FDriveEcsWorld& W)
			{
				W.RemoveComponent_Immediate<TComponent>(E);
			});
			return;
		}

		RemoveComponent_Immediate<TComponent>(E);
	}

	int32 GetStorageCount() const { return Storages.Num(); }

private:
	void DestroyEntity_Immediate(const FDriveEntity& E)
	{
		if (!EntityMgr.IsAlive(E))
		{
			return;
		}

        for (auto& Pair : Storages)
        {
            if (Pair.Value.IsValid())
            {
                Pair.Value->RemoveForEntity(E.Id);
            }
        }

		EntityMgr.Destroy(E);
	}

	template<typename TComponent>
	void AddOrSetComponent_Immediate(const FDriveEntity& E, const TComponent& Value)
	{
		if (!EntityMgr.IsAlive(E))
		{
			return;
		}
		GetOrCreateStorage<TComponent>().AddOrSet(E, Value);
	}

	template<typename TComponent>
	void RemoveComponent_Immediate(const FDriveEntity& E)
	{
		if (!EntityMgr.IsAlive(E))
		{
			return;
		}
		TDriveComponentStorage<TComponent>* Storage = FindStorageMutable<TComponent>();
		if (Storage)
		{
			Storage->Remove(E);
		}
	}

	template<typename TComponent>
	static FName TypeId()
	{
		return FName(UTF8_TO_TCHAR(typeid(TComponent).name()));
	}

	template<typename TComponent>
	TDriveComponentStorage<TComponent>& GetOrCreateStorage()
	{
		const FName Key = TypeId<TComponent>();
		if (TUniquePtr<IDriveComponentStorage>* Found = Storages.Find(Key))
		{
			return *static_cast<TDriveComponentStorage<TComponent>*>(Found->Get());
		}

		TUniquePtr<IDriveComponentStorage> NewStorage = MakeShared<TDriveComponentStorage<TComponent>>();
		IDriveComponentStorage* Raw = NewStorage.Get();
		Storages.Add(Key, MoveTemp(NewStorage));
		return *static_cast<TDriveComponentStorage<TComponent>*>(Raw);
	}

	template<typename TComponent>
	TDriveComponentStorage<TComponent>* FindStorageMutable()
	{
		const FName Key = TypeId<TComponent>();
		if (TUniquePtr<IDriveComponentStorage>* Found = Storages.Find(Key))
		{
			return static_cast<TDriveComponentStorage<TComponent>*>(Found->Get());
		}
		return nullptr;
	}

	template<typename TComponent>
	const TDriveComponentStorage<TComponent>* FindStorage() const
	{
		const FName Key = TypeId<TComponent>();
		if (const TUniquePtr<IDriveComponentStorage>* Found = Storages.Find(Key))
		{
			return static_cast<const TDriveComponentStorage<TComponent>*>(Found->Get());
		}
		return nullptr;
	}

private:
	bool bInFixedTick = false;

	FDriveEntityManager EntityMgr;
	FDriveEcsCommandBuffer CmdBuffer;

	TMap<FName, TSharedPtr<IDriveComponentStorage>> Storages;
};
