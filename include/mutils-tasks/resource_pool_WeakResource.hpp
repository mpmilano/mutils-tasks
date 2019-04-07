#pragma once
#include "resource_pool_declarations.hpp"

namespace mutils{
	namespace resource_pool{
		
		template<typename T, typename... Args>
		class WeakResource{
		public:
			using decls = resource_pool_declarations<T,Args...>;
			using index_owner = typename decls::index_owner;
			using state = typename decls::state;
			using rented_resource = typename decls::rented_resource;
		private:
			std::shared_ptr<const index_owner> index_preference;
			std::shared_ptr<state> parent;
			std::weak_ptr<rented_resource> rsource;

		public:
			LockedResource<T,Args...> lock(Args && ... a);
			bool is_locked() const;
			LockedResource<T,Args...> acquire_if_locked() const;
			WeakResource(const WeakResource&) = delete;
			WeakResource(WeakResource&&);
			explicit WeakResource(std::shared_ptr<state> parent);
			WeakResource& operator=(const LockedResource<T,Args...>& lr);
			explicit WeakResource(const LockedResource<T,Args...>& lr);

			template<typename T2, typename... Args2>
			friend class LockedResource;
		};
	}
}
