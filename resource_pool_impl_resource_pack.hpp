#pragma once

namespace mutils{

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
	ResourcePool<T,Args...>::resource_pack::resource_pack(std::size_t ind)
		:resource{nullptr},index{ind}{}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::resource_pack::resource_pack(resource_pack&& o)
		:resource(std::move(o.resource)),
		 index(o.index),
		 initialized(o.initialized){}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::preferred_resource::preferred_resource(std::size_t ind)
		:ResourcePool<T,Args...>::resource_pack(ind){}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::preferred_resource::preferred_resource(preferred_resource&& ind)
		:ResourcePool<T,Args...>::resource_pack(std::forward<preferred_resource>(ind))
	{}

	template<typename T, typename... Args>
	std::shared_ptr<typename ResourcePool<T,Args...>::rented_resource>
	ResourcePool<T,Args...>::preferred_resource::borrow(std::shared_ptr<state> s, Args && ... a){
		lock l{mut};
		initialize_if_needed(*this,s->builder,std::forward<Args>(a)...);
		if (this->resource){
			return std::make_shared<rented_preferred>(std::move(this->resource),s,this->index);
		}
		else throw ResourceInvalidException{};
	}

	template<typename T, typename... Args>
	void
	ResourcePool<T,Args...>::preferred_resource::remove_from_free_list(){
		in_free_list = false;
	}

}
