#pragma once
#include "resource_pool_rented_resources.hpp"

namespace mutils{
	namespace resource_pool{

	template<typename T, typename... Args>
	overdrawn<T,Args...>::overdrawn(std::shared_ptr<state> sp, std::unique_ptr<T> tp, Args&&... aaa)
		:rented_resource<T,Args...>(std::move(tp), sp, std::forward<Args>(aaa)...){
		auto over_by = ++this->parent->overdrawn_count;
		while (this->parent->max_overdraw < over_by) {
			auto cand = this->parent->max_overdraw.load();
			if (this->parent->max_overdraw.compare_exchange_strong(cand,over_by)){
				break;
			}
		}
		this->parent->number_overdraws++;
		this->parent->sum_overdraws.fetch_add(over_by);
	}
	
	template<typename T, typename... Args>
	overdrawn<T,Args...>::~overdrawn(){
		this->before_delete();
		this->parent->overdrawn_count--;
	}
}
}
