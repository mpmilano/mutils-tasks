#pragma once
#include "resource_pool_declarations.hpp"

namespace mutils{
	namespace resource_pool{

		template<typename T, typename... Args>
		struct rented_resource {
			using decls = resource_pool_declarations<T,Args...>;
			using state = typename decls::state;
			std::unique_ptr<T> t;
			std::shared_ptr<state> parent;
			virtual std::pair<std::size_t,resource_type> which__resource_type() const = 0;
			bool deleted{false};
			void before_delete();
			virtual ~rented_resource();
			rented_resource(std::unique_ptr<T> t, std::shared_ptr<state> parent, Args&&...);
		};

		template<typename T, typename... Args>
		struct rented_preferred : public rented_resource<T,Args...>{
			using this_p = rented_preferred*;
			using decls = resource_pool_declarations<T,Args...>;
			using size_type = typename decls::size_type;
			using state = typename decls::state;
			using lock = typename decls::lock;
			
			this_p& who_owns_me;
			const size_type index;
			rented_preferred(std::unique_ptr<T> t, std::shared_ptr<state> parent, size_type indx, this_p& who_owns_me, Args&&...);
			std::pair<std::size_t,resource_type> which__resource_type() const;
			static std::pair<std::size_t,resource_type> _resource_type();
			~rented_preferred();
		};

		template<typename T, typename... Args>
		struct overdrawn : public rented_resource<T,Args...>{
			using decls = resource_pool_declarations<T,Args...>;
			using state = typename decls::state;

			overdrawn(std::shared_ptr<state> sp, std::unique_ptr<T> tp, Args&&...);
			std::pair<std::size_t,resource_type> which__resource_type() const;
			static std::pair<std::size_t,resource_type> _resource_type();
			~overdrawn();
		};

		template<typename T, typename... Args>
		struct rented_spare : public rented_resource<T,Args...>{
			using decls = resource_pool_declarations<T,Args...>;
			using size_type = typename decls::size_type;
			using state = typename decls::state;
			
			const size_type index;
			rented_spare(std::shared_ptr<state> sp, std::unique_ptr<T> tp, size_type indx, Args&&...);
			std::pair<std::size_t,resource_type> which__resource_type() const;
			static std::pair<std::size_t,resource_type> _resource_type();
			~rented_spare();
		};
	}}
