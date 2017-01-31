#pragma once
#include "resource_pool_index_owner.hpp"

namespace mutils {
	namespace resource_pool{
		
		template<typename T, typename... Args>
		index_owner<T,Args...>::index_owner(const size_type indx, std::shared_ptr<state> parent)
			:indx(indx),parent(parent){}
		
		/*
			template<typename T, typename... Args>
			ResourcePool<T,Args...>::index_owner::index_owner(index_owner&& o)
			:indx(o.indx),parent(o.parent){
			o.parent.reset();
			}//*/
		
		template<typename T, typename... Args>
		index_owner<T,Args...>::~index_owner(){
			if (parent){
				assert(indx < parent->max_resources);
				parent->recycled_indices.add(indx);
			}
		}
		
}

}
