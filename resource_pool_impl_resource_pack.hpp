#pragma once

namespace mutils{

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::resource_pack::resource_pack(std::size_t ind, resource_type type)
		:resource{nullptr},index{ind},type{type}{}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::resource_pack::resource_pack(resource_pack&& o)
		:resource(std::move(o.resource)),
		 index(o.index),
		 initialized(o.initialized),type{o.type}{}
}
