#pragma once

namespace mutils{

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_resource::rented_resource(std::unique_ptr<T> t, std::shared_ptr<state> parent)
		:t(std::move(t)),parent(parent){}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_resource::~rented_resource(){}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_preferred::rented_preferred(std::unique_ptr<T> t, std::shared_ptr<state> parent, size_type index)
		:rented_resource(std::move(t),parent),index(index){
		assert(!parent->preferred_resources.at(index).resource);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_preferred::~rented_preferred(){
		assert(index < this->parent->max_resources);
		lock l{this->parent->preferred_resources.at(index).mut};
		this->parent->preferred_resources.at(index).resource = std::move(this->t);
		if (!this->parent->preferred_resources.at(index).in_free_list){
			this->parent->preferred_resources.at(index).in_free_list = true;
			this->parent->free_resources.add(&this->parent->preferred_resources.at(index));
		}
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_spare::rented_spare(std::shared_ptr<state> parent, std::unique_ptr<T> t, size_type index)
		:rented_resource(std::move(t),parent),index(index){
		assert(!parent->spare_resources.at(index).resource);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::rented_spare::~rented_spare(){
		this->parent->spare_resources.at(index).resource = std::move(this->t);
		this->parent->free_resources.add(&this->parent->spare_resources.at(index));
	}
}
