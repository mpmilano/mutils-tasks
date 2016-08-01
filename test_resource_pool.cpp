#include <iostream>
#include "resource_pool.hpp"

using namespace mutils;

int main(){
	{
	using LockedResource = typename ResourcePool<int>::LockedResource;
	using WeakResource = typename ResourcePool<int>::WeakResource;
	ResourcePool<int> pool{3,[](){return new int{0};}};
	auto prefer_1 = [&](){
		auto my_int = pool.acquire();
		assert(*my_int == 0);
		*my_int = 1;
		WeakResource ret{my_int};
		assert(*ret.lock() == 1);
		return ret;
	}();
	auto prefer_2 = [&](){
		auto my_int_2 = pool.acquire();
		*my_int_2 = 2;
		WeakResource ret{my_int_2};
		assert(*ret.lock() == 2);
		return ret;
	}();
	auto prefer_3 = [&](){
		auto my_int_3 = pool.acquire();
		*my_int_3 = 3;
		WeakResource ret{my_int_3};
		assert(*ret.lock() == 3);
		return ret;
	}();
	assert(pool.dbg_pool_full());
	assert(*prefer_2.lock() != 0);
	assert(*prefer_1.lock() != 0);
	assert(*prefer_3.lock() == 3);
	assert(*prefer_2.lock() == 2);
	assert(*prefer_1.lock() == 1);
	assert(pool.dbg_pool_full());
	WeakResource orphan{[&](const auto &, const auto &, const auto&){
			assert(*pool.acquire() == 0);
			return pool.acquire();
		}(prefer_1.lock(), prefer_2.lock(),prefer_3.lock())};
	auto val = *orphan.lock();
	assert(val == 1 || val == 2 || val == 3);
	assert(*prefer_2.lock() == 2);
	assert(*prefer_1.lock() == 1);
	auto locked_orphan = orphan.lock();
	assert(*locked_orphan == 1 || *locked_orphan == 2 || *locked_orphan == 3);
	if (*locked_orphan == 3){
		assert(*prefer_2.lock() == 2);
		assert(*prefer_1.lock() == 1);
		auto tmp = *prefer_3.lock();
		assert(tmp == 1 || tmp == 2);
	}
	if (*locked_orphan == 2){
		assert(*prefer_1.lock() == 1);
		assert(*prefer_2.lock() == 1 || *prefer_2.lock() == 3);
		assert(*prefer_3.lock() == 3);
	}
	if (*locked_orphan == 1){
		assert(*prefer_1.lock() == 2 || *prefer_2.lock() == 3);
		assert(*prefer_2.lock() == 2);
		assert(*prefer_3.lock() == 3);
	}
	assert(*pool.acquire() != 0);
}
}
