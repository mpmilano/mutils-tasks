#include "delay_thread.hpp"
#include "blockingconcurrentqueue.h"
#include <chrono>
#include <functional>
#include <thread>

using namespace std;
using namespace chrono;

namespace mutils {
namespace delay_thread {
class delay_thread {
private:
    struct delayed_action {
        function<void()> task;
        high_resolution_clock::time_point time;
    };
    nanoseconds delay_amount;
    moodycamel::BlockingConcurrentQueue<delayed_action> q;
    std::atomic<bool> destroying{false};
    thread t{[this] {
        auto& q = this->q;
        delayed_action action;
        while(bool success = q.wait_dequeue_timed(action, delay_amount)) {
            if(destroying)
                break;
            else {
                while(high_resolution_clock::now() < action.time) {
                    if(destroying)
                        break;
                    this_thread::sleep_for(1ms);
                }
                if(destroying)
                    break;
                action.task();
            }
        }
    }};
    delay_thread() = default;
    ~delay_thread() {
        destroying = true;
        t.join();
    }

public:
    template <typename T>
    void set_delay(T&& t) {
        delay_amount = t;
    }
    void submit_action(std::function<void()> task) {
        q.enqueue(delayed_action{task, high_resolution_clock::now() + delay_amount});
    }
    static delay_thread& instance() {
        static delay_thread ret;
        return ret;
    }
};
void set_delay(std::chrono::seconds t) {
    delay_thread::instance().set_delay(t);
}
void set_delay_highres(std::chrono::nanoseconds t) {
    delay_thread::instance().set_delay(t);
}
void delay_action(std::function<void()> a) {
    delay_thread::instance().submit_action(a);
}

}  // namespace delay_thread
}  // namespace mutils