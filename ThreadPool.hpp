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


//ThreadPool adheres to the TaskPool design restrictions, so that its interface matches ProcessPool.

namespace mutils{

	template<typename Mem1, typename Mem2, typename Ret, typename... Arg>
	class ThreadPool_impl : public TaskPool_impl<ThreadPool_impl<Mem1,Mem2,Ret,Arg...>,Mem1,Mem2,Ret,Arg...>{

		struct memories{
			std::shared_ptr<Mem1> thread_local_memory;
			std::vector<std::shared_ptr<Mem2> > simulated_memory;
			int next_simulated{0};

			memories() = default;
			memories(const memories&) = default;
		};
		
		std::shared_ptr<std::vector<memories> > memory;
		int next_enhanced_index{0};
	
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
			  memory(new std::vector<memories>(this->tp ? limit : 1)),
			  thread_max(limit)
			{
			assert(this->tp);
			assert(memory->size() == thread_max);
			for (std::size_t i = 0; i < (this->tp ? limit : 1); ++i){
				indices.add(i);
			}

			for (auto i : indices.iterable_copy()){
				auto &mem = memory->at(i);
				mem.simulated_memory.emplace_back(nullptr);
				auto j = 0;
				auto &sm = mem.simulated_memory.at(j);
				auto sm_index = i + (thread_max * j);
				init(i,mem.thread_local_memory,sm_index,sm);
				assert(mem.thread_local_memory);
				assert(mem.simulated_memory.at(0));
				assert(memory->at(i).thread_local_memory);
			}

			assert(indices.size() == thread_max);
			assert(indices.iterable_copy().front() == 0);
			
			assert(memory->at(0).thread_local_memory);
			assert(memory->at(0).simulated_memory.at(0));
		}

	private:
		std::size_t mem_count(const std::vector<memories>& m) const {
			auto enhance_progress = (thread_max + next_enhanced_index-1) % thread_max;
			assert(enhance_progress < thread_max);
			assert(m.size() == thread_max);
			auto max_sim_size = m.at(enhance_progress).simulated_memory.size();
			auto ret = (enhance_progress + 1) + ((max_sim_size-1) * thread_max);
			assert([&](){
					auto count = 0;
					for (auto &mem : m){
						count += mem.simulated_memory.size();
					}
					assert(count == ret);
					return count == ret;
				}());
			return ret;
		}
	public:

		std::size_t mem_count() const {
			return mem_count(*memory);
		}

		virtual void increase_mem(std::size_t howmuch) {
			using namespace std;
			using namespace chrono;
			auto oldmem = mem_count();
			using namespace std;
			auto new_mem =
				std::make_shared<std::vector<memories> >(*memory);
			assert(new_mem->size() == memory->size());
			assert(memory->at(0).thread_local_memory);
			assert(new_mem->at(0).thread_local_memory);
			auto end = (next_enhanced_index + howmuch);
			for (int _i = next_enhanced_index; _i < end; ++_i){
				auto i = _i % thread_max;
				auto &mem = new_mem->at(i);
				assert(mem.simulated_memory.size() > 0);
				mem.simulated_memory.emplace_back(nullptr);
				int j = mem.simulated_memory.size() - 1;
				assert(!mem.simulated_memory.at(j));
				this->init(i, mem.thread_local_memory,
						   i + (j*thread_max),mem.simulated_memory.at(j));
                                next_enhanced_index = (next_enhanced_index+1) % thread_max;
			}
			assert(mem_count(*new_mem) == howmuch + oldmem);
			memory.swap(new_mem);
			assert(mem_count() == howmuch + oldmem);
		}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			auto this_sp = this->this_sp;
			assert(this_sp.get() == this);
			auto fun =
				[this_sp,command,arg...](int) -> std::unique_ptr<Ret>{
				auto index = this_sp->indices.pop();
				AtScopeEnd ase{[&](){
						this_sp->indices.add(index);
					}};
				
				auto &mem = this_sp->memory->at(index);
				auto j = mem.next_simulated;
				mem.next_simulated = (mem.next_simulated + 1) % mem.simulated_memory.size();
				auto &cmnd = this_sp->behaviors.at(command);
				assert(j < mem.simulated_memory.size());
				auto &simmem = mem.simulated_memory.at(j);
				try{
					return heap_copy(cmnd(
										 index,mem.thread_local_memory,
                                                        index + (j * this_sp->thread_max),simmem,
										 arg...));
				}
				catch(...){
					return heap_copy(this_sp->onException(std::current_exception()));
				}
			};
			if (this->tp){
				if (this->tp->n_idle() > 0)
					return this->tp->push(fun);
				else return GlobalPool::push(fun);
			}
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
