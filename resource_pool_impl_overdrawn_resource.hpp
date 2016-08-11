#pragma once

namespace mutils{


	template<typename T, typename... Args>
	ResourcePool<T,Args...>::overdrawn::overdrawn(std::shared_ptr<state> sp, std::unique_ptr<T> tp)
		:rented_resource(std::move(tp), sp){
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
	ResourcePool<T,Args...>::overdrawn::~overdrawn(){
		this->parent->overdrawn_count--;
	}
}
