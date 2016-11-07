#include "eventfd.hpp"
#include <sys/eventfd.h>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
using namespace std;

namespace mutils {

	eventfd::eventfd(bool block_on_first_wait)
		:fd(::eventfd((block_on_first_wait ? 0 : 1),0)){}
	eventfd::~eventfd(){
		close(fd);
	}
	void eventfd::wait(){
		buf_t buf{0};
#ifndef NDEBUG
		auto result =
#endif
			read(fd,&buf,sizeof(buf));
		assert(result == sizeof(buf));
	}

	void eventfd::clear(){
		notify();
		wait();
	}
	
	void eventfd::notify(){
		buf_t buf{1};
#ifndef NDEBUG
		auto result =
#endif
			write(fd,&buf,sizeof(buf));
		assert(result == sizeof(buf));
	}

	int eventfd::underlying_fd() const { return fd;}
}
