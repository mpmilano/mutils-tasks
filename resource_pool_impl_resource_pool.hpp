#pragma once

namespace mutils{
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::ResourcePool(size_type max_resources, const decltype(state::builder) &builder)
		:_state(new state(max_resources,builder)){}
	
	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::acquire(Args && ... a){
		return _state->acquire_new_preference(_state, std::forward<Args>(a)...);
	}
}
