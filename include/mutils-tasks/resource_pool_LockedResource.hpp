#pragma once
#include "resource_pool_resource_type.hpp"
#include "resource_pool_declarations.hpp"

namespace mutils{

	namespace resource_pool {

		template<typename T, typename... Args>
		class LockedResource{
		public:
			using decls = resource_pool_declarations<T,Args...>;
			using index_owner = typename decls::index_owner;
			using state = typename decls::state;
			using rented_resource = typename decls::rented_resource;
		private:
			std::shared_ptr<const index_owner> index_preference;
			std::shared_ptr<state> parent;
			std::shared_ptr<rented_resource> rsource;
			LockedResource(const LockedResource&);
			
		public:
			LockedResource(std::shared_ptr<const index_owner> indx,
						   std::shared_ptr<state> parent,
						   std::shared_ptr<rented_resource> rsource);
			LockedResource(LockedResource&& o);

			const T * operator->() const;
			T* operator->();

			T& operator*();
			const T& operator&() const;

			LockedResource lock(const Args &...);
			bool is_locked() const;
			LockedResource acquire_if_locked() const;
			std::pair<std::size_t,resource_type> which__resource_type() const;

			LockedResource clone();
			WeakResource<T, Args...> weak();
			explicit LockedResource(const WeakResource<T, Args...>& wr);

			template <typename T2, typename... Args2>
			friend class WeakResource;
		};
		
	}
}
