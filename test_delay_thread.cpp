#include "delay_thread.hpp"
#include <iostream>
#include <thread>

using namespace mutils;
using namespace delay_thread;
using namespace std;
using namespace chrono;
int main() {
    set_delay(2s);
    delay_action([] { std::cout << "this is a simple test" << std::endl; });
    delay_action([] { std::cout << "this is a simple test" << std::endl; });
    set_delay(3s);
    delay_action([] { std::cout << "this is a simple test" << std::endl; });
    this_thread::sleep_for(4s);
}