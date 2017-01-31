#pragma once
#include "resource_pool_declarations.hpp"

namespace mutils{
	namespace resource_pool {

		template<typename T, typename... Args>
		struct resource_pack{
			using decls = resource_pool_declarations<T,Args...>;
			using rented_resource = typename decls::rented_resource;
			using state = typename decls::state;
			
			std::unique_ptr<T> resource;
			const std::size_t index;
			bool initialized{false};
			resource_pack(std::size_t ind);
			resource_pack(resource_pack&&);
			virtual std::shared_ptr<rented_resource> borrow(std::shared_ptr<state>, Args && ... a) = 0;
			virtual void remove_from_free_list() = 0;
		protected: virtual ~resource_pack() {};
		};

		template<typename T, typename... Args>
		struct preferred_resource : public resource_pack<T,Args...>{
			using decls = resource_pool_declarations<T,Args...>;
			using rented_resource = typename decls::rented_resource;
			using state = typename decls::state;
			using lock = typename decls::lock;
			
			std::mutex mut;
			rented_preferred<T,Args...>* who_owns_me{nullptr};
			bool in_free_list{false};
			preferred_resource(std::size_t ind);
			preferred_resource(preferred_resource&&);
			void remove_from_free_list();
			std::shared_ptr<rented_resource> borrow(std::shared_ptr<state>, Args && ... a);
		};

		template<typename T, typename... Args>
		struct spare_resource : public resource_pack<T,Args...>{
			using decls = resource_pool_declarations<T,Args...>;
			using rented_resource = typename decls::rented_resource;
			using state = typename decls::state;
			
			spare_resource(std::size_t ind);
			spare_resource(spare_resource&&);
			void remove_from_free_list(){}
			std::shared_ptr<rented_resource> borrow(std::shared_ptr<state>, Args && ... a);
		};
	}}
