#pragma once
#include <typeinfo>
#include "resource_pool_rented_resources.hpp"

namespace mutils{
	namespace resource_pool{
	namespace resource_pool_helpers{
		template<typename T>
		auto on_release(T* t) -> decltype(t->onRelease()){
			return t->onRelease();
		}

		template<typename>
		auto on_release(void*) -> void {}

		template<typename T, typename... Args>
		auto on_acquire(T* t, Args&&... args)
			-> decltype(t->onAcquire(std::declval<Args>()...))
		{
			return t->onAcquire(std::forward<Args>(args)...);
		}

		
		template<typename, typename... Args>
		auto on_acquire(void*,const Args&...) -> void {}
	}

	template<typename T, typename... Args>
	rented_resource<T,Args...>::rented_resource(std::unique_ptr<T> t, std::shared_ptr<state> parent, Args&&... aaa)
		:t(std::move(t)),parent(parent){
		assert(this->t);
		resource_pool_helpers::template on_acquire<T,Args...>(this->t.get(),std::forward<Args>(aaa)...);
	}

	template<typename T, typename... Args>
	rented_resource<T,Args...>::~rented_resource(){
		assert(deleted);
	}
	
	template<typename T, typename... Args>
	void rented_resource<T,Args...>::before_delete(){
		deleted = true;
		assert(t);
		resource_pool_helpers::template on_release<T>(t.get());
	}

	template<typename T, typename... Args>
	rented_preferred<T,Args...>::rented_preferred(std::unique_ptr<T> t, std::shared_ptr<state> parent, size_type index, this_p& who_owns_me, Args&&... aaa)
		:rented_resource<T,Args...>(std::move(t),parent,std::forward<Args>(aaa)...),
		 who_owns_me(who_owns_me),
		 index(index){
		who_owns_me = this;
		assert(!parent->preferred_resources.at(index).resource);
	}

	template<typename T, typename... Args>
	rented_preferred<T,Args...>::~rented_preferred(){
		this->before_delete();
		assert(index < this->parent->max_resources);
		lock l{this->parent->preferred_resources.at(index).mut};
		who_owns_me = nullptr;
		preferred_resource<T,Args...>& source = this->parent->preferred_resources.at(index);
		source.resource = std::move(this->t);
		if (!source.in_free_list){
			source.in_free_list = true;
			this->parent->free_resources.enqueue(&source);
		}
	}

	template<typename T, typename... Args>
	rented_spare<T,Args...>::rented_spare(std::shared_ptr<state> parent, std::unique_ptr<T> t, size_type index, Args&&... aaa)
		:rented_resource<T,Args...>(std::move(t),parent,std::forward<Args>(aaa)...),
		 index(index){
		assert(!parent->spare_resources.at(index).resource);
	}

	template<typename T, typename... Args>
	rented_spare<T,Args...>::~rented_spare(){
		this->before_delete();
		this->parent->spare_resources.at(index).resource = std::move(this->t);
		this->parent->free_resources.enqueue(&this->parent->spare_resources.at(index));
	}

	template<typename T, typename... Args>
	std::pair<std::size_t,resource_type> rented_preferred<T,Args...>::which_resource_type() const {
		return resource_type();
	}
	template<typename T, typename... Args>
	std::pair<std::size_t,resource_type> rented_spare<T,Args...>::which_resource_type() const {
		return resource_type();
	}
	template<typename T, typename... Args>
	std::pair<std::size_t, resource_type> overdrawn<T,Args...>::which_resource_type() const {
		return resource_type();
	}

	template<typename T, typename... Args>
	std::pair<std::size_t, resource_type> rented_preferred<T,Args...>::resource_type(){
		auto &tmp = typeid(rented_preferred);
		return std::make_pair(tmp.hash_code(),resource_type::preferred);
	}

	template<typename T, typename... Args>
	std::pair<std::size_t, resource_type> rented_spare<T,Args...>::resource_type(){
		auto &tmp = typeid(rented_spare);
		return std::make_pair(tmp.hash_code(),resource_type::spare);
	}

	template<typename T, typename... Args>
	std::pair<std::size_t, resource_type> overdrawn<T,Args...>::resource_type(){
		auto &tmp = typeid(overdrawn);
		return std::make_pair(tmp.hash_code(),resource_type::overdrawn);
	}
	
	}}
