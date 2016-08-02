#pragma once

namespace mutils {
		template<typename T, typename... Args>
	ResourcePool<T,Args...>::state::state(size_type max_resources, const decltype(builder) &builder)
		:max_resources(max_resources),
		 builder(builder){
		for (std::size_t i = 0; i < max_resources; ++i){
			resources.emplace_back(i);
		}
	}
	
	template<typename T, typename... Args>
	bool ResourcePool<T,Args...>::state::pool_full() const {
		assert(this);
		return max_resources < (current_max_index + 1)
			&& recycled_indices.size() == 0;
	}

	template<typename T, typename... Args>	
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::state::acquire_no_preference(std::shared_ptr<state> _this, Args && ... a){
		//don't even try to get a preference
		const static std::function<resource_pack* ()> nullfun = []()-> resource_pack* {return nullptr;};
		auto *cand = _this->free_resources.
			template build_or_pop<resource_pack*>(nullfun);
		while (cand){
			lock l{cand->mut};
			if (cand->resource)
				return
					LockedResource{
					std::move(cand->resource),
						_this,
						std::make_shared<index_owner>(cand->index,_this)};
			else cand = _this->free_resources.build_or_pop(nullfun);
		}
		//whoops, entirely full!
		//just build a one-off resource
		return LockedResource{std::unique_ptr<T>{_this->builder(std::forward<Args>(a)...)},_this};
	}
	
	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::state::acquire_new_preference(std::shared_ptr<state> _this, Args && ... a){
		//no preference!
		if (!_this->pool_full()){
			//try to get a preference!
			auto my_preference = (_this->recycled_indices.size() > 0 ?
								  _this->recycled_indices.
								  template build_or_pop<size_type>(
									  [&cmi = _this->current_max_index](){
										  return cmi.fetch_add(1);})
								  : _this->current_max_index.fetch_add(1));
			if (my_preference < _this->max_resources){
				//have a preference!
				return acquire_with_preference(
					_this,
					std::make_shared<index_owner>(my_preference,_this),
					std::forward<Args>(a)...);
			}
			else {
				//you lost, sorry. Do the no-preference-available route.
				return acquire_no_preference(_this,std::forward<Args>(a)...);
			}
		}
		else {
			return acquire_no_preference(_this,std::forward<Args>(a)...);
		}
	}

	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::state::acquire_with_preference(std::shared_ptr<state> _this, std::shared_ptr<const index_owner> preference, Args && ... a){
		//preference!
		auto &my_resource = _this->resources.at(preference->indx);
		lock l{my_resource.mut};
		if (!my_resource.initialized){
			my_resource.initialized = true;
			my_resource.resource.reset(_this->builder(std::forward<Args>(a)...));
		}
		if (my_resource.resource){
			return LockedResource{std::move(my_resource.resource),
					_this,preference};
		}
		else return acquire_no_preference(_this,std::forward<Args>(a)...);
	}
}
