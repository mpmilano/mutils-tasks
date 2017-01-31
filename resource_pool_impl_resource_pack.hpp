#pragma once
#include "resource_pool_rented_resources.hpp"
#include "resource_pool_resource_packs.hpp"

namespace mutils{
	namespace resource_pool{
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
	resource_pack<T,Args...>::resource_pack(std::size_t ind)
		:resource{nullptr},index{ind}{}

	template<typename T, typename... Args>
	resource_pack<T,Args...>::resource_pack(resource_pack&& o)
		:resource(std::move(o.resource)),
		 index(o.index),
		 initialized(o.initialized){}

	template<typename T, typename... Args>
	preferred_resource<T,Args...>::preferred_resource(std::size_t ind)
		:resource_pack<T,Args...>(ind){}

	template<typename T, typename... Args>
	preferred_resource<T,Args...>::preferred_resource(preferred_resource&& ind)
		:resource_pack<T,Args...>(std::forward<preferred_resource>(ind))
	{}

	template<typename T, typename... Args>
	std::shared_ptr<typename preferred_resource<T,Args...>::rented_resource>
	preferred_resource<T,Args...>::borrow(std::shared_ptr<state> s, Args && ... a){
		lock l{mut};
		initialize_if_needed(*this,s->builder,std::forward<Args>(a)...);
		if (this->resource){
			return std::make_shared<rented_preferred<T,Args...> >(std::move(this->resource),s,this->index,who_owns_me,std::forward<Args>(a)...);
		}
		else {
			assert(who_owns_me);
			throw ResourceInvalidException{};
		}
	}

	template<typename T, typename... Args>
	void
	preferred_resource<T,Args...>::remove_from_free_list(){
		in_free_list = false;
	}

	}}
