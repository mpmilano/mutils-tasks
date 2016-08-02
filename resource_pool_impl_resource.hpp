#pragma once

namespace mutils{

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::resource::resource(std::unique_ptr<T> t, std::shared_ptr<state> parent, size_type index)
		:t(std::move(t)),parent(parent),index(index){
		assert(!parent->resources.at(index).resource);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::resource::~resource(){
		assert(index < parent->max_resources);
		lock l{parent->resources.at(index).mut};
		parent->resources.at(index).resource = std::move(t);
		parent->free_resources.add(&parent->resources.at(index));
	}
}
