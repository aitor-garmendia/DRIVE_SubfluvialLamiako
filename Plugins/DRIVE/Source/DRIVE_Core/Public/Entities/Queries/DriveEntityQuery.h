#pragma once

#include "CoreMinimal.h"

namespace Drive::Query
{
	namespace Detail
	{
		template<typename StoreT, typename EntityIdT>
		static FORCEINLINE bool HasComponent(const StoreT& Store, const EntityIdT& EntityId)
		{
			if constexpr (requires { Store.Has(EntityId); })
			{
				return Store.Has(EntityId);
			}
			else if constexpr (requires { Store.Contains(EntityId); })
			{
				return Store.Contains(EntityId);
			}
			else if constexpr (requires { Store.Find(EntityId); })
			{
				return Store.Find(EntityId) != nullptr;
			}
			else
			{
				static_assert(sizeof(StoreT) == 0, "Component store must provide Has(EntityId) or Contains(EntityId) or Find(EntityId).");
				return false;
			}
		}

		template<typename StoreT, typename EntityIdT>
		static FORCEINLINE auto TryGetPtr(StoreT& Store, const EntityIdT& EntityId)
		{
			if constexpr (requires { Store.TryGet(EntityId); })
			{
				return Store.TryGet(EntityId); 
			}
			else if constexpr (requires { Store.Get(EntityId); })
			{
				return &Store.Get(EntityId);   
			}
			else
			{
				static_assert(sizeof(StoreT) == 0, "Component store must provide TryGet(EntityId) or Get(EntityId).");
				return (decltype(&Store.Get(EntityId)))nullptr;
			}
		}

		template<typename StoreT, typename EntityIdT>
		static FORCEINLINE auto TryGetPtr(const StoreT& Store, const EntityIdT& EntityId)
		{
			if constexpr (requires { Store.TryGet(EntityId); })
			{
				return Store.TryGet(EntityId);
			}
			else if constexpr (requires { Store.Get(EntityId); })
			{
				return &Store.Get(EntityId);
			}
			else
			{
				static_assert(sizeof(StoreT) == 0, "Component store must provide TryGet(EntityId) or Get(EntityId).");
				return (decltype(&Store.Get(EntityId)))nullptr;
			}
		}

		template<typename EntityIdT, typename... StoreTs>
		static FORCEINLINE bool HasAll(const EntityIdT& EntityId, const StoreTs&... Stores)
		{
			return (HasComponent(Stores, EntityId) && ...);
		}
	} // namespace Detail


	template<typename EntityIdT, typename... StoreTs>
	class TEntityView
	{
	public:
		TEntityView(TConstArrayView<EntityIdT> InEntities, StoreTs&... InStores): Entities(InEntities), Stores(InStores...)
		{
		}

		template<typename FuncT>
		FORCEINLINE void ForEach(FuncT&& Func)
		{
			for (const EntityIdT& E : Entities)
			{
				ApplyIfMatches(E, Func);
			}
		}

		template<typename WherePredT, typename FuncT>
		FORCEINLINE void ForEachWhere(WherePredT&& WherePred, FuncT&& Func)
		{
			for (const EntityIdT& E : Entities)
			{
				ApplyIfMatchesWhere(E, WherePred, Func);
			}
		}

	private:
		template<typename FuncT>
		FORCEINLINE bool ApplyIfMatches(const EntityIdT& E, FuncT&& Func)
		{
			return ApplyIfMatchesImpl(E, Forward<FuncT>(Func), TMakeIntegerSequence<int32, sizeof...(StoreTs)>{});}

		template<typename WherePredT, typename FuncT>
		FORCEINLINE bool ApplyIfMatchesWhere(const EntityIdT& E, WherePredT&& WherePred, FuncT&& Func)
		{
			return ApplyIfMatchesWhereImpl(E, Forward<WherePredT>(WherePred), Forward<FuncT>(Func), TMakeIntegerSequence<int32, sizeof...(StoreTs)>{});
		}

		template<typename FuncT, int32... Is>
		FORCEINLINE bool ApplyIfMatchesImpl(const EntityIdT& E, FuncT&& Func, TIntegerSequence<int32, Is...>)
		{
			if (!Detail::HasAll(E, GetStore<Is>()...))
			{
				return false;
			}

			void* Ptrs[] = { (void*)Detail::TryGetPtr(GetStore<Is>(), E)... };
			for (void* P : Ptrs)
			{
				if (P == nullptr)
				{
					return false;
				}
			}

			Func(E, (*Detail::TryGetPtr(GetStore<Is>(), E))...);
			return true;
		}

		template<typename WherePredT, typename FuncT, int32... Is>
		FORCEINLINE bool ApplyIfMatchesWhereImpl(const EntityIdT& E, WherePredT&& WherePred, FuncT&& Func, TIntegerSequence<int32, Is...>)
		{
			if (!Detail::HasAll(E, GetStore<Is>()...))
			{
				return false;
			}

			void* Ptrs[] = { (void*)Detail::TryGetPtr(GetStore<Is>(), E)... };
			for (void* P : Ptrs)
			{
				if (P == nullptr)
				{
					return false;
				}
			}

			if (WherePred(E, (*Detail::TryGetPtr(GetStore<Is>(), E))...))
			{
				Func(E, (*Detail::TryGetPtr(GetStore<Is>(), E))...);
			}

			return true;
		}

		template<int32 Index>
		FORCEINLINE auto& GetStore()
		{
			return Stores.template Get<Index>();
		}

		template<int32 Index>
		FORCEINLINE const auto& GetStore() const
		{
			return Stores.template Get<Index>();
		}

	private:
		TConstArrayView<EntityIdT> Entities;
		TTuple<StoreTs&...> Stores;
	};

	template<typename EntityIdT, typename... StoreTs>
	FORCEINLINE TEntityView<EntityIdT, StoreTs...> MakeView(TConstArrayView<EntityIdT> Entities, StoreTs&... Stores)
	{
		return TEntityView<EntityIdT, StoreTs...>(Entities, Stores...);
	}

	template<typename EntityIdT, typename... StoreTs>
	FORCEINLINE TEntityView<EntityIdT, StoreTs...> MakeView(const TArray<EntityIdT>& Entities, StoreTs&... Stores)
	{
		return TEntityView<EntityIdT, StoreTs...>(MakeArrayView(Entities), Stores...);
	}
} // namespace Drive::Query