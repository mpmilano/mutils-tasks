#pragma once

namespace mutils {
		template<typename T, typename... Args>
		ResourcePool<T,Args...>::state::state(size_type max_resources, size_type max_spares, const decltype(builder) &builder)
		:max_resources(max_resources),
		 builder(builder){
		for (std::size_t i = 0; i < max_resources; ++i){
			preferred_resources.emplace_back(i);
		}
		for (std::size_t i = 0; i < max_spares; ++i){
			spare_resources.emplace_back(i);
			free_resources.add(&spare_resources.at(i));
		}
	}
	
	template<typename T, typename... Args>
	bool ResourcePool<T,Args...>::state::preferred_full() const {
		assert(this);
		return max_resources < (current_max_index + 1)
			&& recycled_indices.size() == 0;
	}

	template<typename T, typename... Args>	
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::state::acquire_no_preference(std::shared_ptr<state> _this, Args && ... a){
		//don't even try to get a preference
		auto *cand = _this->free_resources.pop();
		while (cand){
			try {
				cand->remove_from_free_list();
				return LockedResource{nullptr, _this, cand->borrow(_this,std::forward<Args>(a)...)};
			}
			catch(const ResourceInvalidException&){
				cand = _this->free_resources.pop();
			}
		}
		//whoops, entirely full!
		//just build a one-off resource
		return LockedResource{nullptr, _this, std::make_shared<overdrawn>(_this,std::unique_ptr<T>{_this->builder(std::forward<Args>(a)...)})};
	}
	
	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::state::acquire_new_preference(std::shared_ptr<state> _this, Args && ... a){
		//no preference!
		if (!_this->preferred_full()){
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
		auto &my_resource = _this->preferred_resources.at(preference->indx);
		try {
			return LockedResource{
				preference,
					_this,
					my_resource.borrow(_this,std::forward<Args>(a)...)};
		}
		catch (const ResourceInvalidException&){
			return acquire_no_preference(_this,std::forward<Args>(a)...);
		}
	}

	/*
	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::state::~state(){} //*/
}
