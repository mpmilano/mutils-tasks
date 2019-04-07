#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include "resource_pool_declarations.hpp"
#include "resource_pool_LockedResource.hpp"
#include "resource_pool_WeakResource.hpp"

namespace mutils{

	struct ResourceInvalidException : public std::exception {
		const char* what() const noexcept{
			return "ResourceInvalidException";
		}
	};

	//resources are initialized lazily.
	//three sorts of resources: preferred try and reclaim the resource they had before,
	//spare resources don't care,
	//and overdrawn resources are destroyed immediately after use.
	//I don't think there's any way to destroy resources at this point without
	//also destroying the pool itself. 
	
	template<typename T, typename... Args>
	class ResourcePool{

	public:

		using decls = typename resource_pool::resource_pool_declarations<T,Args...>;
		
	private:
		using state = typename decls::state;
		std::shared_ptr<state> _state;
		
	public:

		auto dbg_leak_state() { return _state;}

		using size_type = typename decls::size_type;
		ResourcePool(size_type max_resources, size_type max_spares, const decltype(state::builder) &builder, bool allow_overdraws=true);
		
		ResourcePool(const ResourcePool&) = delete;

		//find the definition of this in resource_pool_LockedResource.hpp
		using LockedResource = typename decls::LockedResource;
		using WeakResource = typename decls::WeakResource;
		
		LockedResource acquire(Args && ... a);
		WeakResource acquire_weak();

		size_type number_free_resources() const;
		bool preferred_full() const { return _state->preferred_full();}
		
		~ResourcePool();

		//you probably won't need these aliases, but they could come in handy all the same
		using rented_preferred = typename decls::rented_preferred;
		using rented_spare = typename decls::rented_spare;
		using overdrawn = typename decls::overdrawn;
	};
}

#include "resource_pool_impl.hpp"
