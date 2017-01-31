#pragma once

namespace mutils{
	namespace resource_pool{

	template<typename T, typename... Args>
	spare_resource<T,Args...>::spare_resource(std::size_t ind)
		:resource_pack<T,Args...>(ind){}

	template<typename T, typename... Args>
	spare_resource<T,Args...>::spare_resource(spare_resource&& ind)
		:resource_pack<T,Args...>(std::forward<spare_resource>(ind))
	{}

	template<typename T, typename... Args>
	std::shared_ptr<typename spare_resource<T,Args...>::rented_resource >
	spare_resource<T,Args...>::borrow(std::shared_ptr<state> s, Args && ... a){
		initialize_if_needed(*this,s->builder,std::forward<Args>(a)...);
		assert(this->resource);
		return std::make_shared<rented_spare<T,Args...> >(s,std::move(this->resource),this->index,std::forward<Args>(a)...);
	}
	}}
