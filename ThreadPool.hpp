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
#include <mutils.hpp>


//ThreadPool adheres to the TaskPool design restrictions, so that its interface matches ProcessPool.

namespace mutils{

	template<typename Mem1, , typename Mem2, typename Ret, typename... Arg>
	class ThreadPool_impl : public TaskPool_impl<ThreadPool_impl<Mem1,Mem2,Ret,Arg...>,Mem1,Mem2,Ret,Arg...>{

		std::shared_ptr<std::vector<std::shared_ptr<Mem1> > > thread_local_memory;
		std::shared_ptr<std::vector<std::shared_ptr<Mem2> > > simulated_memory;
		
		SafeSet<int> indices;
		const int thread_max;
	
	public:

		using super_t = TaskPool_impl<ThreadPool_impl<Mem1,Mem2,Ret,Arg...>,Mem1,Mem2,Ret,Arg...>;

		using init_f = typename super_t::init_f;
		using action_f = typename super_t::action_f;
		using exception_f = typename super_t::exception_f;
	
		ThreadPool_impl (std::shared_ptr<ThreadPool_impl> &pp,
						 const init_f &init,
						 std::vector<action_f > beh,
						 int limit,
						 exception_f onException
			):super_t(pp,init,beh,limit,onException),
			  thread_local_memory(new std::vector<std::shared_ptr<Mem> >(this->tp ? limit : 1)),
			  simulated_memory(new std::vector<std::shared_ptr<Mem> >(this->tp ? limit : 1)),
			  thread_max(limit)
			{
			assert(this->tp);
			for (int i = 0; i < (this->tp ? limit : 1); ++i)
				indices.add(i);

			for (auto i : indices.iterable_copy()){
				init(i,thread_local_memory->at(i),i,simulated_memory->at(i));
				assert(thread_local_memory->at(i) && simulated_memory->at(i));
			}
		}

		std::size_t mem_count() const {
			return simulated_memory->size();
		}

		virtual void increase_mem(std::size_t howmuch) {
			using namespace std;
			auto first_index = simulated_memory->size();
			auto new_mem =
				std::make_shared<std::vector<std::shared_ptr<Mem2> > >(first_index + howmuch);
			new_mem->insert(new_mem->begin(),simulated_memory->begin(),simulated_memory->end());
			for (int i = first_index; i < first_index + howmuch; ++i){
				this->init(i % thread_max, thread_local_memory->at(i % thread_max),
						   i,new_mem->at(i));
			}
			atomic_store(&simulated_memory,new_mem);
			for (int i = first_index; i < first_index + howmuch; ++i){
				indices.add(i);
			}
		}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			auto this_sp = this->this_sp;
			assert(this_sp.get() == this);
			auto fun =
				[this_sp,command,arg...](int) -> std::unique_ptr<Ret>{
				const auto thread_max = this_sp->thread_max;
				auto mem_indx = this_sp->indices.pop();
				AtScopeEnd ase{[&](){this_sp->indices.add(mem_indx);}};
				try{
					assert(this_sp->thread_local_memory->at(mem_indx % thread_max));
					return heap_copy(
						this_sp->behaviors.at(command)(
							mem_indx%thread_max,this_sp->thread_local_memory->at(mem_indx%thread_max),
							mem_indx,this_sp->simulated_memory->at(mem_indx),
							mem_indx,arg...));
				}
				catch(...){
					return heap_copy(this_sp->onException(std::current_exception()));
				}
			};
			if (this->tp) return this->tp->push(fun);
			else {
				int id = std::hash<std::thread::id>{}(std::this_thread::get_id());
				return std::async(std::launch::deferred, std::move(fun),id);
			}
		}

		virtual ~ThreadPool_impl(){
			assert(this->pool_alive == false);
			std::cout << "threadpool destroyed" << std::endl;
		}
	};
	
	template<typename Mem, typename Ret, typename... Arg>
	using ThreadPool = TaskPool<ThreadPool_impl<Mem,Ret,Arg...>,Mem,Ret,Arg...>;

}
