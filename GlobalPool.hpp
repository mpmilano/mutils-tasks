#pragma once
#include "ctpl_stl.h"
#include <iostream>

struct GlobalPool{
private:
	GlobalPool(){
		std::cout << "Global thread count: " << pool.size() << std::endl;
	}
public:
	GlobalPool(const GlobalPool&) = delete;
	//access to this is already thread safe; no need to double-lock.
	ctpl::thread_pool pool{MAX_THREADS};
	static GlobalPool inst;
	template<typename... Args>
	static auto push(Args && ... args){
		return inst.pool.push(std::forward<Args>(args)...);
	}
};

