#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include "SafeSet.hpp"

namespace mutils{

	struct ResourceInvalidException : public std::exception {
		const char* what() const noexcept{
			return "ResourceInvalidException";
		}
	};
	
	template<typename T, typename... Args>
	class ResourcePool{

	public:
		class LockedResource;
		class WeakResource;
		struct index_owner;
		struct rented_resource;
	private:
		struct state;
		
		struct resource_pack{

			std::unique_ptr<T> resource;
			const std::size_t index;
			bool initialized{false};
			resource_pack(std::size_t ind);
			resource_pack(resource_pack&&);
			virtual std::shared_ptr<rented_resource> borrow(std::shared_ptr<state>, Args && ... a) = 0;
			virtual void remove_from_free_list() = 0;
		protected: virtual ~resource_pack() {};
		};

	public:
		struct rented_preferred;
	private:
		struct preferred_resource : public resource_pack{
			std::mutex mut;
			rented_preferred* who_owns_me{nullptr};
			bool in_free_list{false};
			preferred_resource(std::size_t ind);
			preferred_resource(preferred_resource&&);
			void remove_from_free_list();
			std::shared_ptr<rented_resource> borrow(std::shared_ptr<state>, Args && ... a);
		};
		
		struct spare_resource : public resource_pack{
			spare_resource(std::size_t ind);
			spare_resource(spare_resource&&);
			void remove_from_free_list(){}
			std::shared_ptr<rented_resource> borrow(std::shared_ptr<state>, Args && ... a);
		};
		
		using resources_vector = std::vector<preferred_resource>;
		using size_type = typename resources_vector::size_type;
		using lock = std::unique_lock<std::mutex>;
		
		struct state {
			resources_vector preferred_resources;
			const size_type max_resources;
			std::atomic<size_type> current_max_index{0};
			SafeSet<size_type> recycled_indices;
			SafeSet<resource_pack*> free_resources;
			std::vector<spare_resource> spare_resources;
			const std::function<T* (Args...)> builder;
			//for debugging, mostly.
			std::atomic_ullong overdrawn_count{0};
			std::atomic_ullong max_overdraw{0};
			std::atomic_ullong number_overdraws{0};
			std::atomic_ullong sum_overdraws{0};
			
			state(size_type max_resources, size_type max_spares, const decltype(builder) &builder);
			//~state();

			bool preferred_full() const;

			static LockedResource acquire_no_preference(std::shared_ptr<state> _this, Args && ... a);
			static LockedResource acquire_new_preference(std::shared_ptr<state> _this, Args && ... a);
			
			static LockedResource acquire_with_preference(std::shared_ptr<state> _this, std::shared_ptr<const index_owner> preference, Args && ...);
		};
		
		std::shared_ptr<state> _state;
		
	public:

		auto dbg_leak_state() { return _state;}
		
		ResourcePool(size_type max_resources, size_type max_spares, const decltype(state::builder) &builder);
		
		ResourcePool(const ResourcePool&) = delete;

		struct index_owner{
			const size_type indx;
			std::shared_ptr<state> parent;
			index_owner(const size_type indx, std::shared_ptr<state> parent);
			index_owner(const index_owner&) = delete;
			//index_owner(index_owner&&);
			~index_owner();
		};

		struct rented_resource {
			std::unique_ptr<T> t;
			std::shared_ptr<state> parent;
			virtual std::pair<std::size_t,std::string> which_resource_type() const = 0;
			bool deleted{false};
			void before_delete();
			virtual ~rented_resource();
			rented_resource(std::unique_ptr<T> t, std::shared_ptr<state> parent, Args&&...);
		};

		struct rented_preferred : public rented_resource{
			using this_p = rented_preferred*;
			this_p& who_owns_me;
			const size_type index;
			rented_preferred(std::unique_ptr<T> t, std::shared_ptr<state> parent, size_type indx, this_p& who_owns_me, Args&&...);
			std::pair<std::size_t,std::string> which_resource_type() const;
			static std::pair<std::size_t,std::string> resource_type();
			~rented_preferred();
		};

		struct overdrawn : public rented_resource{
			overdrawn(std::shared_ptr<state> sp, std::unique_ptr<T> tp, Args&&...);
			std::pair<std::size_t,std::string> which_resource_type() const;
			static std::pair<std::size_t,std::string> resource_type();
			~overdrawn();
		};

		struct rented_spare : public rented_resource{
			const size_type index;
			rented_spare(std::shared_ptr<state> sp, std::unique_ptr<T> tp, size_type indx, Args&&...);
			std::pair<std::size_t,std::string> which_resource_type() const;
			static std::pair<std::size_t,std::string> resource_type();
			~rented_spare();
		};
		
		class LockedResource{
		private:
			std::shared_ptr<const index_owner> index_preference;
			std::shared_ptr<state> parent;
			std::shared_ptr<rented_resource> rsource;
			LockedResource(const LockedResource&);
			
		public:
			LockedResource(std::shared_ptr<const index_owner> indx,
						   std::shared_ptr<state> parent,
						   std::shared_ptr<rented_resource> rsource);
			LockedResource(LockedResource&& o);

			const T * operator->() const;
			T* operator->();

			T& operator*();
			const T& operator&() const;

			LockedResource lock(const Args &...);
			bool is_locked() const;
			LockedResource acquire_if_locked() const;
			std::pair<std::size_t,std::string> which_resource_type() const;

			LockedResource clone();
			explicit LockedResource(const WeakResource& wr);
			friend class WeakResource;

		};
		class WeakResource{
		private:
			std::shared_ptr<const index_owner> index_preference;
			std::shared_ptr<state> parent;
			std::weak_ptr<rented_resource> rsource;

		public:
			LockedResource lock(Args && ... a);
			bool is_locked() const;
			LockedResource acquire_if_locked() const;
			WeakResource(const WeakResource&) = delete;
			WeakResource(WeakResource&&);
			WeakResource& operator=(const LockedResource& lr);
			explicit WeakResource(const LockedResource& lr);
			friend class LockedResource;
		};
		
		LockedResource acquire(Args && ... a);

		size_type number_free_resources() const;
		bool preferred_full() const { return _state->preferred_full();}
		
		~ResourcePool();
	};
}

#include "resource_pool_impl.hpp"
