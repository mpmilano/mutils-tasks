#pragma once
#include <chrono>
#include <functional>

namespace mutils {
namespace delay_thread {
void set_delay(std::chrono::seconds);
void set_delay_highres(std::chrono::nanoseconds);
void delay_action(std::function<void()>);
}  // namespace delay_thread
}  // namespace mutils