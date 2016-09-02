#pragma once
#include <mutex>
#include <condition_variable>

namespace mutils{

	struct condition_held{
		std::mutex &m;
		std::condition_variable &v;
		std::function<bool()> wake;
		std::unique_lock<std::mutex> l;
		condition_held(std::mutex &m,
					   std::condition_variable &v,
					   std::function<bool()> wake)
			:m(m),v(v),wake(wake),l(m){
			v.wait(m,wake);
		}
		
		~condition_held(){
			l.unlock();
			v.notify_all();
		}
	};
	
	struct condition_variable{
		std::mutex m;
		std::condition_variable v;
		condition_variable() = default;
		condition_held wait(std::function<bool()> wake){
			return condition_held{m,v,wake};
		}
	};
}
