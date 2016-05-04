#pragma once
#include <memory>
#include <future>

//generic interface implemented by ProcessPool, ThreadPool, and NetworkedProcessPool.  

namespace mutils{

	template<typename>
	struct default_on_exn;
	
	template<>
	struct default_on_exn<std::string> {
		static constexpr auto value = "Exception Occurred!";
	};
	
	template<typename Impl, typename Mem1, typename Mem2, typename Ret, typename... Arg>
	class TaskPool;
	
	template<typename Impl, typename Mem1, typename Mem2, typename Ret, typename... Arg>
	class TaskPool_impl {
	public:
		using init_f = std::function<void (int, std::shared_ptr<Mem1>&, int, std::shared_ptr<Mem2>&)>;
		using action_f = std::function<Ret (int, std::shared_ptr<Mem1>, int, std::shared_ptr<Mem2>&, Arg...)>;
		using exception_f = std::function<Ret (std::exception_ptr)>;

	protected:
		const int limit;
		std::unique_ptr<ctpl::thread_pool> tp;
		init_f init;
		std::vector<action_f> behaviors;
		exception_f onException;
		bool pool_alive;
		std::shared_ptr<Impl> &this_sp;

		TaskPool_impl (std::shared_ptr<Impl> &pp,
					   const init_f &init,
					   std::vector<action_f> beh,
					   int limit,
					   exception_f onException
			):limit(limit),tp(limit > 0 ? new ctpl::thread_pool{limit} : nullptr),
			  init(init),behaviors(beh),onException(onException),pool_alive(true),this_sp(pp){}
		
		//it is intended for the constructor to take the same types as TaskPool
	public:
		virtual std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg) = 0;
		virtual std::size_t mem_count() const = 0;
		
		virtual void increase_mem(std::size_t howmuch) = 0;

		void set_mem_to(std::size_t value){
			assert(value >= mem_count());
			if (value > mem_count()){
				increase_mem(mem_count() - value);
			}
		}

	private:
		SafeSet<std::pair<std::thread::id, int> > pending_set;
		void register_pending(std::thread::id id, int name){
			pending_set.add(std::make_pair(id,name));
		}
		void remove_pending(std::thread::id id, int name){
			pending_set.remove(std::make_pair(id,name));
		}
		
	public:

		void print_pending(){
			for (auto p : pending_set.iterable_copy()){
				std::cout << "thread id: " << p.first << " pid: " << p.second << std::endl;
			}
		}
		
		virtual ~TaskPool_impl(){}
		template<typename Mem2, typename Impl2, typename Ret2, typename... Arg2>
		friend class TaskPool;
	};
	
	template<class Impl, typename Ret, typename... Arg>
	class TaskPool {
		std::shared_ptr<Impl > inst;
	public:

		using init_f = typename Impl::init_f;
		using action_f = typename Impl::action_f;
		using exception_f = typename Impl::exception_f;

		//The "memory" cell is guaranteed to be passed in queue order; we make the *longest*
		//possible duration elapse 

		TaskPool (
			init_f init_mem,
			std::vector<action_f> beh,
			int limit = 200,
			exception_f onExn = [](std::exception_ptr exn){
				  try {
					  assert(exn);
					  std::rethrow_exception(exn);
				  }
				  catch (...){
					  return default_on_exn<Ret>::value;
				  }
				  assert(false && "exn handler called with no currrent exception?");
			  })
			:inst(new Impl(inst,init_mem,beh,limit,onExn)){}
		
		std::future<std::unique_ptr<Ret> > launch(int command, const Arg & ... arg){
			return inst->launch(command,arg...);
		}

		//increases counts of "Mem" *without* increasing underlying number of threads
		void increase_mem(std::size_t count){
			inst->increase_mem(count);
		}

		//if count is at or above the current mem, increases mem to reach count.
		void set_mem_to(std::size_t count){
			inst->set_mem_to(count);
		}
		
		void print_pending(){
			return inst->print_pending();
		}
		
		virtual ~TaskPool(){
			inst->pool_alive = false;
			std::cout << "sent task pool destroy" << std::endl;
		}
	};
	
}
