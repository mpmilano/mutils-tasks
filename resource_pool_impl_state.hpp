#pragma once
#include "resource_pool_state.hpp"
#include "resource_pool_LockedResource.hpp"
#include "resource_pool_rented_resources.hpp"

namespace mutils {
	namespace resource_pool {
		template<typename T, typename... Args>
		pool_state<T,Args...>::pool_state(typename pool_state<T,Args...>::size_type max_resources,
																			typename pool_state<T,Args...>::size_type max_spares,
																			const decltype(builder) &builder, bool allow_overdraws)
			:allow_overdraws(allow_overdraws),
			 max_resources(max_resources),
		 builder(builder){
		for (std::size_t i = 0; i < max_resources; ++i){
			preferred_resources.emplace_back(i);
		}
		for (std::size_t i = 0; i < max_spares; ++i){
			spare_resources.emplace_back(i);
		}
		for (std::size_t i = 0; i < max_spares; ++i){
			free_resources.add(&spare_resources.at(i));
		}
	}
	
	template<typename T, typename... Args>
	bool pool_state<T,Args...>::preferred_full() const {
		assert(this);
		return max_resources < (current_max_index + 1)
			&& recycled_indices.size() == 0;
	}

	template<typename T, typename... Args>	
	typename pool_state<T,Args...>::LockedResource pool_state<T,Args...>::acquire_no_preference(std::shared_ptr<pool_state> _this, Args && ... a){
		//don't even try to get a preference
#define acquire_no_preference_internal_23847892784(pop_type)	\
		auto *cand = _this->free_resources.pop_type();						\
		while (cand){																		\
			try {																					\
				cand->remove_from_free_list();							\
				assert(cand);																										\
				return LockedResource{nullptr, _this, cand->borrow(_this,std::forward<Args>(a)...)}; \
			}																																	\
			catch(const ResourceInvalidException&){														\
				cand = _this->free_resources.pop_type();												\
			}																																	\
		}																																		\

		{
			acquire_no_preference_internal_23847892784(pop);
		}
		if (_this->allow_overdraws){
			//whoops, entirely full!
			//just build a one-off resource
			return LockedResource{nullptr, _this, std::make_shared<overdrawn>(_this,std::unique_ptr<T>{_this->builder(std::forward<Args>(a)...)},std::forward<Args>(a)...)};
		}
		else {
			//do the acquire song-and-dance again, except this time block on pop when empty
			++_this->number_waiters;
			acquire_no_preference_internal_23847892784(pop_blocking);
		}
		struct this_is_not_possible_exn {};
		throw this_is_not_possible_exn{};
	}
	
	template<typename T, typename... Args>
	typename pool_state<T,Args...>::LockedResource pool_state<T,Args...>::acquire_new_preference(std::shared_ptr<pool_state> _this, Args && ... a){
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
	typename pool_state<T,Args...>::LockedResource pool_state<T,Args...>::acquire_with_preference(std::shared_ptr<pool_state> _this, std::shared_ptr<const index_owner> preference, Args && ... a){
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
	typename pool_state<T,Args...>::~state(){} //*/
}
}
