#pragma once

namespace mutils {
	
	struct eventfd {
		const int fd{};

		eventfd(bool block_on_first_wait = true);
		~eventfd();

		using buf_t = unsigned long long;
		static_assert(sizeof(buf_t) == 8,"error: eventfd returns exactly 8 bytes");

		void wait();

		void clear();

		void notify();
		
		int underlying_fd() const; 
	};

}
