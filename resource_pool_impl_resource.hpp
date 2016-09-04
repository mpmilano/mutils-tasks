#pragma once
#include <typeinfo>

namespace mutils{
	
	namespace resource_pool_helpers{
		template<typename T>
		auto on_release(T* t) -> decltype(t->onRelease()){
			return t->onRelease();
		}

		template<typename>
		auto on_release(void*) -> void {}

		template<typename T, typename... Args>
		auto on_acquire(T* t, Args&&... args)
			-> decltype(t->onAcquire(std::forward<Args>(args)...))
		{
			return t->onAcquire(std::forward<Args>(args)...);
		}

		template<typename, typename... Args>
		auto on_acquire(void*,const Args&...) -> void {}
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_resource::rented_resource(std::unique_ptr<T> t, std::shared_ptr<state> parent, Args&&... aaa)
		:t(std::move(t)),parent(parent){
		resource_pool_helpers::template on_acquire<T>(this->t.get(),std::forward<Args>(aaa)...);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_resource::~rented_resource(){
		resource_pool_helpers::template on_release<T>(t.get());
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_preferred::rented_preferred(std::unique_ptr<T> t, std::shared_ptr<state> parent, size_type index, this_p& who_owns_me, Args&&... aaa)
		:rented_resource(std::move(t),parent,std::forward<Args>(aaa)...),
		 who_owns_me(who_owns_me),
		 index(index){
		who_owns_me = this;
		assert(!parent->preferred_resources.at(index).resource);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_preferred::~rented_preferred(){
		assert(index < this->parent->max_resources);
		lock l{this->parent->preferred_resources.at(index).mut};
		who_owns_me = nullptr;
		preferred_resource& source = this->parent->preferred_resources.at(index);
		source.resource = std::move(this->t);
		if (!source.in_free_list){
			source.in_free_list = true;
			this->parent->free_resources.add(&source);
		}
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_spare::rented_spare(std::shared_ptr<state> parent, std::unique_ptr<T> t, size_type index, Args&&... aaa)
		:rented_resource(std::move(t),parent,std::forward<Args>(aaa)...),
		 index(index){
		assert(!parent->spare_resources.at(index).resource);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_spare::~rented_spare(){
		this->parent->spare_resources.at(index).resource = std::move(this->t);
		this->parent->free_resources.add(&this->parent->spare_resources.at(index));
	}

	template<typename T, typename... Args>
	std::pair<std::size_t,std::string> ResourcePool<T,Args...>::rented_preferred::which_resource_type() const {
		auto &tmp = typeid(*this);
		return std::make_pair(tmp.hash_code(),std::string{tmp.name()});
	}
	template<typename T, typename... Args>
	std::pair<std::size_t,std::string> ResourcePool<T,Args...>::rented_spare::which_resource_type() const {
		auto &tmp = typeid(*this);
		return std::make_pair(tmp.hash_code(),std::string{tmp.name()});
	}
	template<typename T, typename... Args>
	std::pair<std::size_t,std::string> ResourcePool<T,Args...>::overdrawn::which_resource_type() const {
		auto &tmp = typeid(*this);
		return std::make_pair(tmp.hash_code(),std::string{tmp.name()});
	}

	template<typename T, typename... Args>
	std::pair<std::size_t,std::string> ResourcePool<T,Args...>::rented_preferred::resource_type(){
		auto &tmp = typeid(rented_preferred);
		return std::make_pair(tmp.hash_code(),std::string{tmp.name()});
	}

	template<typename T, typename... Args>
	std::pair<std::size_t,std::string> ResourcePool<T,Args...>::rented_spare::resource_type(){
		auto &tmp = typeid(rented_spare);
		return std::make_pair(tmp.hash_code(),std::string{tmp.name()});
	}

	template<typename T, typename... Args>
	std::pair<std::size_t,std::string> ResourcePool<T,Args...>::overdrawn::resource_type(){
		auto &tmp = typeid(overdrawn);
		return std::make_pair(tmp.hash_code(),std::string{tmp.name()});
	}
	
}
