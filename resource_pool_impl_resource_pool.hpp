#pragma once

namespace mutils{
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::ResourcePool(size_type max_resources, const decltype(state::builder) &builder)
		:_state(new state(max_resources,builder)){}
	
	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::acquire(Args && ... a){
		return _state->acquire_new_preference(_state, std::forward<Args>(a)...);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::~ResourcePool(){
		std::cout << "Resource pool usage statistics: " << std::endl
				  << "Number overdraws: " << _state->number_overdraws << std::endl
				  << "Max overdraw: " << _state->max_overdraw << std::endl
				  << "Average overdraw: "
				  << (_state->sum_overdraws /
					  (1 + _state->number_overdraws)) << std::endl;
	}
}
