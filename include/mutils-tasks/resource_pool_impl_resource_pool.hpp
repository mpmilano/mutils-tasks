#pragma once
#include "resource_pool.hpp"

namespace mutils{
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::ResourcePool(size_type max_resources, size_type max_spares, const decltype(state::builder) &builder, bool allow_overdraws)
		:_state(new state(max_resources,max_spares,builder,allow_overdraws)){}
	
	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::acquire(Args && ... a){
		return _state->acquire_new_preference(_state, std::forward<Args>(a)...);
	}

	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::WeakResource ResourcePool<T,Args...>::acquire_weak(){
		return WeakResource(_state);
	}

	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::size_type ResourcePool<T,Args...>::number_free_resources() const {
		assert(false && "I am guessing this was implemented at some point, right?");
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::~ResourcePool(){
#ifndef NDEBUG
		std::cout << "Resource pool usage statistics: " << std::endl
							<< "Number waiters: " << _state->number_waiters << std::endl
				  << "Number overdraws: " << _state->number_overdraws << std::endl
				  << "Max overdraw: " << _state->max_overdraw << std::endl
				  << "Average overdraw: "
				  << (_state->sum_overdraws /
					  (1 + _state->number_overdraws)) << std::endl;
#endif
	}
}
