#pragma once
#include "SafeSet.hpp"
#include "ctpl_stl.h"
#include "SerializationSupport.hpp"
#include <unistd.h>
#include <signal.h>
#include <exception>
#include <vector>
#include "compile-time-tuple.hpp"
#include "TaskPool.hpp"
#include "GlobalPool.hpp"
#include <mutils.hpp>


//GloballyBackedExecutor adheres to the TaskPool design restrictions, so that its interface matches ProcessPool.

namespace mutils{

	template<typename Mem, typename Ret, typename... Arg>
	class GloballyBackedExecutor_impl : public TaskPool_impl<GloballyBackedExecutor_impl<Mem,Ret,Arg...>,Mem,Ret,Arg...>{
		using Super = TaskPool_impl<GloballyBackedExecutor_impl<Mem,Ret,Arg...>,Mem,Ret,Arg...>;

		std::vector<std::shared_ptr<Mem> > remember_these;
		//pop() will block if no indices are available.
		SafeSet<int> indices;
	
	public:
	
		GloballyBackedExecutor_impl (std::shared_ptr<GloballyBackedExecutor_impl> &pp,
						 const typename Super::init_f &init,
						 std::vector<typename Super::action_f> beh,
						 int /*will always be 0*/,
						 typename Super::exception_f onException
			):Super(pp,init,beh,0,onException){
		}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			auto this_sp = this->this_sp;
			auto fun =
				[this_sp,command,arg...](int) -> std::unique_ptr<Ret>{
				auto mem_indx = this_sp->indices.pop();
				AtScopeEnd ase{[&](){this_sp->indices.add(mem_indx);}};
				try{
					assert(this_sp->remember_these.at(mem_indx));
					return heap_copy(this_sp->behaviors.at(command)(mem_indx,this_sp->remember_these.at(mem_indx),arg...));
				}
				catch(...){
					return heap_copy(this_sp->onException(std::current_exception()));
				}
			};
			return GlobalPool::push(fun);
		}

		std::size_t mem_count() const {
			return remember_these.size();
		}

		void increase_mem(std::size_t howmuch) {
			auto max = howmuch + mem_count();
			for (auto i = mem_count(); i < max; ++i){
				remember_these.emplace_back();
				this->init(i,remember_these.back());
				indices.add(i);
			}
		}

		virtual ~GloballyBackedExecutor_impl(){
			assert(this->pool_alive == false);
			std::cout << "threadpool destroyed" << std::endl;
		}
	};
	
	template<typename Mem, typename Ret, typename... Arg>
	struct GloballyBackedExecutor : public TaskPool<GloballyBackedExecutor_impl<Mem,Ret,Arg...>,Ret,Arg...> {
		using Super = TaskPool<GloballyBackedExecutor_impl<Mem,Ret,Arg...>,Ret,Arg...>;
		
			GloballyBackedExecutor (
				typename Super::init_f init_mem,
				std::vector<typename Super::action_f> beh,
				typename Super::exception_f onExn = Super::default_exception)
				:Super(init_mem,beh,0,onExn){}
	};

}
