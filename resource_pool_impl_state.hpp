#pragma once

namespace mutils {
		template<typename T, typename... Args>
		ResourcePool<T,Args...>::state::state(size_type max_resources, size_type max_spares, const decltype(builder) &builder)
		:max_resources(max_resources),
		 builder(builder){
		for (std::size_t i = 0; i < max_resources; ++i){
			resources.emplace_back(i,resource_type::preferred);
		}
		for (std::size_t i = 0; i < max_spares; ++i){
			spare_resources.emplace_back(i,resource_type::spare);
			free_resources.add(&spare_resources.at(i));
		}
	}
	
	template<typename T, typename... Args>
	bool ResourcePool<T,Args...>::state::pool_full() const {
		assert(this);
		return max_resources < (current_max_index + 1)
			&& recycled_indices.size() == 0;
	}

	namespace {
		template<typename resource_pack, typename F, typename... Args>
		void initialize_if_needed(resource_pack& cand, const F& builder, Args && ... a){
			if (!cand.initialized){
				cand.initialized = true;
				cand.resource.reset(builder(std::forward<Args>(a)...));
			}
		}
	}

	template<typename T, typename... Args>	
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::state::acquire_no_preference(std::shared_ptr<state> _this, Args && ... a){
		//don't even try to get a preference
		auto *cand = _this->free_resources.pop();
		while (cand){
			lock l{cand->mut};
			initialize_if_needed(*cand,_this->builder,std::forward<Args>(a)...);
			if (cand->resource){
				if (cand->type == resource_type::preferred)
					return
						LockedResource{
						std::move(cand->resource),
							_this,
							std::make_shared<index_owner>(cand->index,_this)};
				else if (cand->type == resource_type::spare){
					return
						LockedResource{std::move(cand->resource),_this,cand->index};
				}
			}
			else cand = _this->free_resources.pop();
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
		/* try */{
			lock l{my_resource.mut};
			initialize_if_needed(my_resource,_this->builder,std::forward<Args>(a)...);
			if (my_resource.resource){
				return LockedResource{std::move(my_resource.resource),
						_this,preference};
			}
		}
		//else (is implicit from early return)
		return acquire_no_preference(_this,std::forward<Args>(a)...);
	}

	/*
	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::state::~state(){} //*/
}
