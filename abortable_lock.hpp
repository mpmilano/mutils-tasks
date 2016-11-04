#pragma once
#include "better_cv.hpp"
#include <atomic>

namespace mutils{
	
	struct abortable_locked_guardian {
		std::atomic<bool> critical_region_occupied{false};
		//my personal CV, owns its mutex
		condition_variable cv;
		
		struct abortable_mutex{
			abortable_locked_guardian &parent;
			void lock(){
				//WARNING: assuming cv.m is held by us at this point.
				parent.critical_region_occupied = true;
			}
			void unlock(){
				parent.critical_region_occupied = false;
				parent.cv.v.notify_all();
			}
		};
		
		abortable_mutex am{*this};
		
		using lock_t = std::unique_lock<abortable_mutex>;
		
		//Will abort when abort_condition is true,
		//*regardless* of whether we could have gotten the lock too.
		
		std::unique_ptr<lock_t> lock_or_abort(std::function<bool ()> abort_condition){
			auto locked = cv.wait([&]{return !critical_region_occupied || abort_condition();} );
			if (abort_condition()){
				return nullptr;
			}
			else if (!critical_region_occupied){
				return std::make_unique<lock_t>(am);
			}
			else assert(false && "woke up early, but that's not supposed be possible with C++ condition variables");
		}
	};
};
