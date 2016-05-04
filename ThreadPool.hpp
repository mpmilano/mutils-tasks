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

	template<typename Mem1, typename Mem2, typename Ret, typename... Arg>
	class ThreadPool_impl : public TaskPool_impl<ThreadPool_impl<Mem1,Mem2,Ret,Arg...>,Mem1,Mem2,Ret,Arg...>{

		struct memories{
			std::shared_ptr<Mem1> thread_local_memory;
			std::vector<std::shared_ptr<Mem2> > simulated_memory;
			int next_simulated{0};
		};
		
		std::shared_ptr<std::vector<memories> > memory;
		int last_index_enhanced{0};
	
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
			  thread_local_memory(new std::vector<std::shared_ptr<Mem1> >(this->tp ? limit : 1)),
			  simulated_memory(new std::vector<std::shared_ptr<Mem2> >(this->tp ? limit : 1)),
			  thread_max(limit)
			{
			assert(this->tp);
			for (std::size_t i = 0; i < (this->tp ? limit : 1); ++i){
				indices.add(i);
			}

			for (auto i : indices.iterable_copy()){
				auto &mem = memory->at(i);
				for (int j = 0; i < mem.simulated_memory.size(); ++j){
					auto &sm = mem.simulated_memory.at(j);
					auto sm_index = i + (thread_max * j);
					init(i,mem.thread_local_memory,sm_index,sm);
				}
			}
		}

		std::size_t mem_count() const {
			return simulated_memory->size();
		}

		static void generate_modded_range(std::size_t begin, std::size_t end, std::size_t mod){
			
			for (int i = begin; (begin > end ? i > begin || i < end : i < end); i = ((i + 1)%mod)){
				//body
			}
		}

		virtual void increase_mem(std::size_t howmuch) {
			using namespace std;
			auto new_mem =
				std::make_shared<std::vector<memories> >(*memory);
			assert(new_mem->size() == memory->size());
			assert(new_mem->at(0));
			auto end = (last_index_enhanced + howmuch) % thread_max;
			for (int i = last_index_enhanced;
				 (last_index_enhanced > end ? i > last_index_enhanced || i < end : i < end);
				 i = ((i + 1) % thread_max)){
				auto &mem = memory->at(i);
				mem.sm_indices.emplace_back(nullptr);
				int j = sm_indices.size() - 1;
				this->init(i, mem.thread_local_memory,
						   i + (j*thread_max),mem.simulated_memory.at(j));
			}
			static_assert(std::is_same<decltype(new_mem),decltype(simulated_memory)>::value,"");
			simulated_memory.swap(new_mem);
		}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			auto this_sp = this->this_sp;
			assert(this_sp.get() == this);
			auto fun =
				[this_sp,command,arg...](int) -> std::unique_ptr<Ret>{
				auto tl_indx = this_sp->tl_indices.pop();
				auto sm_indx = this_sp->sm_indices.pop();
				
				AtScopeEnd ase{[&](){
						this_sp->tl_indices.add(tl_indx);
						this_sp->sm_indices.add(sm_indx);
					}};
				
				try{
					assert(this_sp->thread_local_memory->at(tl_indx));
					return heap_copy(
						this_sp->behaviors.at(command) (
							tl_indx,this_sp->thread_local_memory->at(tl_indx),
							sm_indx,this_sp->simulated_memory->at(sm_indx),
							arg...));
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
	
	template<typename Mem1, typename Mem2, typename Ret, typename... Arg>
	using ThreadPool = TaskPool<ThreadPool_impl<Mem1,Mem2,Ret,Arg...>,Ret,Arg...>;

}
