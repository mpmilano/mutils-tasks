#pragma once

namespace mutils{

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::spare_resource::spare_resource(std::size_t ind)
		:ResourcePool<T,Args...>::resource_pack(ind){}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::spare_resource::spare_resource(spare_resource&& ind)
		:ResourcePool<T,Args...>::resource_pack(std::forward<spare_resource>(ind))
	{}

	template<typename T, typename... Args>
	std::shared_ptr<typename ResourcePool<T,Args...>::rented_resource>
	ResourcePool<T,Args...>::spare_resource::borrow(std::shared_ptr<state> s, Args && ... a){
		initialize_if_needed(*this,s->builder,std::forward<Args>(a)...);
		assert(this->resource);
		return std::make_shared<rented_spare>(s,std::move(this->resource),this->index);
	}
}
