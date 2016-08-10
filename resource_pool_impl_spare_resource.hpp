#pragma once

namespace mutils{


	template<typename T, typename... Args>
	ResourcePool<T,Args...>::spare_resource::spare_resource(std::shared_ptr<state> sp, std::unique_ptr<T> tp, size_type indx)
		:parent(sp),t(std::move(tp)),index(indx){
		
	}
	
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::spare_resource::~spare_resource(){
		parent->spare_resources.at(index).resource.reset(t.release());
		//want to re-use these first when the list is consulted.
		parent->free_resources.add_front(&parent->spare_resources.at(index));
	}
}
