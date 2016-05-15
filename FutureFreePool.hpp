#pragma once
#include "ctpl_stl.h"
#include "GlobalPool.hpp"
#include <list>
#include <future>
#include <array>

namespace mutils{

	class FutureFreePool_impl {
		static constexpr int bound = 1000;
		std::list<int> available_ids = [](){std::list <int> ret; for (int i =0; i < bound; ++i) ret.push_back(i); return ret;}();
		using lock = std::unique_lock<std::mutex>;
		std::mutex m;
		using fut_type = decltype(GlobalPool::push(std::declval<std::function<void (int)> >()));
		std::array<fut_type,bound> pending;
		std::shared_ptr<FutureFreePool_impl> &this_p;
		int get_pos();
	public:
		FutureFreePool_impl(std::shared_ptr<FutureFreePool_impl> &this_p)
			:this_p(this_p){}
		void launch (std::function<void (int)> fun);
		friend class FutureFreePool;
	};

//TODO: disable when done debugging

	class FutureFreePool{
		//std::shared_ptr<FutureFreePool_impl> inst;
	public:
		FutureFreePool()//:inst(new FutureFreePool_impl(inst)
			{}
		
		void launch (std::function<void (int)> fun){
			std::thread([fun](){fun(-1);}).detach();
			//return inst->launch(fun);
		}

		template<typename T>
		void take (T fut){
			std::shared_ptr<T> hide{new T(std::move(fut))};
			launch([hide](int){});
		}
	};
}
