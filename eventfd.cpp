#include "eventfd.hpp"
#include <sys/eventfd.h>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
using namespace std;

namespace mutils {

	eventfd::eventfd()
		:fd(::eventfd(0,0)){}
	eventfd::~eventfd(){
		close(fd);
	}
	void eventfd::wait(){
		buf_t buf{0};
#ifndef NDEBUG
		auto result =
#endif
			read(fd,&buf,sizeof(buf));
		assert(result == 0);
	}


	void eventfd::notify(){
		buf_t buf{1};
#ifndef NDEBUG
		auto result =
#endif
			write(fd,&buf,sizeof(buf));
		assert(result == 0);
	}

	int eventfd::underlying_fd() const { return fd;}
}
