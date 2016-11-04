#pragma once

namespace mutils {
	
	struct eventfd {
		const int fd{};

		eventfd();
		~eventfd();

		using buf_t = unsigned long long;
		static_assert(sizeof(buf_t) == 8,"error: eventfd returns exactly 8 bytes");

		void wait();

		void notify();
		
		int underlying_fd() const; 
	};

}
