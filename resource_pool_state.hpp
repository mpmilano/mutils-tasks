#pragma once
#include "resource_pool_declarations.hpp"
#include "resource_pool_resource_packs.hpp"

namespace mutils{
	namespace resource_pool {
		template<typename T, typename... Args>
		struct pool_state {
			using decls = resource_pool_declarations<T,Args...>;
			using resources_vector = typename decls::resources_vector;
			using size_type = typename decls::size_type;
			using resource_pack = typename decls::resource_pack;
			using spare_resource = typename decls::spare_resource;
			using LockedResource = typename decls::LockedResource;
			using index_owner = typename decls::index_owner;
			using overdrawn = typename decls::overdrawn;
			
			const bool allow_overdraws;
			resources_vector preferred_resources;
			const size_type max_resources;
			std::atomic<size_type> current_max_index{0};
			SafeSet<size_type> recycled_indices;
			SafeSet<resource_pack*> free_resources;
			std::vector<spare_resource> spare_resources;
			const std::function<T* (Args...)> builder;
			//for debugging, mostly.
			std::atomic_ullong overdrawn_count{0};
			std::atomic_ullong max_overdraw{0};
			std::atomic_ullong number_overdraws{0};
			std::atomic_ullong sum_overdraws{0};
			std::atomic_ullong number_waiters{0};
			
			pool_state(size_type max_resources, size_type max_spares, const decltype(builder) &builder, bool allow_overdraws);
			//~pool_state();

			bool preferred_full() const;

			static LockedResource acquire_no_preference(std::shared_ptr<pool_state> _this, Args && ... a);
			static LockedResource acquire_new_preference(std::shared_ptr<pool_state> _this, Args && ... a);
			
			static LockedResource acquire_with_preference(std::shared_ptr<pool_state> _this,
																										std::shared_ptr<const index_owner> preference, Args && ...);
		};
	}}
