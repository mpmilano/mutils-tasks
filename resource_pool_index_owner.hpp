#pragma once
#include "resource_pool_declarations.hpp"

namespace mutils{
	namespace resource_pool{
		template<typename T, typename... Args>
		struct index_owner{
			using decls = resource_pool_declarations<T,Args...>;
			using size_type = typename decls::size_type;
			using state = typename decls::state;
			
			const size_type indx;
			std::shared_ptr<state> parent;
			index_owner(const size_type indx, std::shared_ptr<state> parent);
			index_owner(const index_owner&) = delete;
			//index_owner(index_owner&&);
			~index_owner();
		};
	}
}
