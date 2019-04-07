#pragma once
#include "resource_pool_resource_type.hpp"

namespace mutils{

	namespace resource_pool {

		template<typename T, typename... Args>
		class LockedResource;

		template<typename T, typename... Args>
		class WeakResource;

		template<typename T, typename... Args>
		struct pool_state;

		template<typename T, typename... Args>
		struct resource_pack;

		template<typename T, typename... Args>
		struct preferred_resource;

		template<typename T, typename... Args>
		struct spare_resource;

		template<typename T, typename... Args>
		struct rented_resource;
		
		template<typename T, typename... Args>
		struct rented_preferred;

		template<typename T, typename... Args>
		struct rented_spare;

		template<typename T, typename... Args>
		struct overdrawn;

		template<typename T, typename... Args>
		struct index_owner;
		
	template<typename T, typename... Args>
	struct resource_pool_declarations{

		using LockedResource = ::mutils::resource_pool::LockedResource<T,Args...>;
		using WeakResource = ::mutils::resource_pool::WeakResource<T,Args...>;
		
		using state = pool_state<T,Args...>;
		using resource_pack = ::mutils::resource_pool::resource_pack<T,Args...>;
		using preferred_resource = ::mutils::resource_pool::preferred_resource<T,Args...>;
		using spare_resource = ::mutils::resource_pool::spare_resource<T,Args...>;
		using rented_resource = ::mutils::resource_pool::rented_resource<T,Args...>;
		using rented_preferred = ::mutils::resource_pool::rented_preferred<T,Args...>;
		using rented_spare = ::mutils::resource_pool::rented_spare<T,Args...>;
		using overdrawn = ::mutils::resource_pool::overdrawn<T,Args...>;

		using index_owner = ::mutils::resource_pool::index_owner<T,Args...>;
	
		using resources_vector = std::vector<preferred_resource>;
		using size_type = typename resources_vector::size_type;
		using lock = std::unique_lock<std::mutex>;

	};
}
}

#include "resource_pool_state.hpp"
