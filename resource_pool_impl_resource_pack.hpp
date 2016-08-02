#pragma once

namespace mutils{

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::resource_pack::resource_pack(std::size_t ind)
				:resource{nullptr},index{ind}{}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::resource_pack::resource_pack(resource_pack&& o)
		:resource(std::move(o.resource)),
		 index(o.index),
		 initialized(o.initialized){}
}
