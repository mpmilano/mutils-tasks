#pragma once

namespace mutils{


	template<typename T, typename... Args>
	ResourcePool<T,Args...>::overdrawn_resource::overdrawn_resource(std::shared_ptr<state> sp, std::unique_ptr<T> tp)
		:parent(sp),t(std::move(tp)){
		auto over_by = ++parent->overdrawn_count;
		while (parent->max_overdraw < over_by) {
			auto cand = parent->max_overdraw.load();
			if (parent->max_overdraw.compare_exchange_strong(cand,over_by)){
				break;
			}
		}
		parent->number_overdraws++;
		parent->sum_overdraws.fetch_add(over_by);
	}
	
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::overdrawn_resource::~overdrawn_resource(){
		parent->overdrawn_count--;
	}
}
