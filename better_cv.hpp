#pragma once
#include <mutex>
#include <condition_variable>

namespace mutils{

	struct condition_held{

		using who_t = std::thread::id*;
		
		std::condition_variable &v;
		std::function<bool()> wake;
		std::unique_lock<std::mutex> l;
		std::thread::id my_id{std::this_thread::get_id()};
		who_t &who;
		condition_held(std::mutex &m,
					   std::condition_variable &v,
					   std::function<bool()> wake,
					   who_t& who
			)
			:v(v),wake(wake),l(m),who(who){
			v.wait(l,wake);
			who = &my_id;
		}
		condition_held(condition_held&& o)
			:v(o.v),
			 wake(std::move(o.wake)),
			 l(std::move(o.l)),
			 my_id(std::this_thread::get_id()),
			 who(o.who)
			{}
		
		~condition_held(){
			who = nullptr;
			l.unlock();
			v.notify_all();
		}
	};
	
	struct condition_variable{
		std::mutex m;
		std::condition_variable v;
		std::thread::id* held_by{nullptr};
		condition_variable() = default;
		condition_held wait(std::function<bool()> wake){
			return condition_held{m,v,wake,held_by};
		}
		condition_variable(const condition_variable&) = delete;
	};
}
