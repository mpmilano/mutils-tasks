#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include "SafeSet.hpp"

namespace mutils{
	
	template<typename T, typename... Args>
	class ResourcePool{
		using resource_pair = std::pair<std::mutex, std::unique_ptr<T> >;
		struct resource_pack{
			std::mutex mut;
			std::unique_ptr<T> resource;
			const std::size_t index;
			bool initialized{false};
			resource_pack(std::size_t ind)
				:resource{nullptr},index{ind}{}
		};
		
		using resources_vector = std::vector<resource_pair>;
		using size_type = typename resources_vector::size_type;
		using lock = std::unique_lock<std::mutex>;

	public:
		struct LockedResource;
		struct WeakResource;
	private:
		
		struct state {
			resources_vector resources;
			const size_type max_resources;
			std::atomic<size_type> current_max_index{0};
			SafeSet<size_type> recycled_indices;
			SafeSet<resource_pair*> free_resources;
			const std::function<T* (Args...)> builder;
			state(size_type max_resources, const decltype(builder) &builder);

			bool pool_full() const;

			static LockedResource acquire_no_preference(std::shared_ptr<state> _this, Args && ... a);
			static LockedResource acquire_new_preference(std::shared_ptr<state> _this, Args && ... a);
			
			static LockedResource acquire_with_preference(std::shared_ptr<state> _this, size_type preference, Args && ...);
		};
		
		std::shared_ptr<state> _state;

		ResourcePool(size_type max_resources, const decltype(state::builder) &builder);
		
		ResourcePool(const ResourcePool&) = delete;
		
	public:

		struct index_owner{
			const size_type indx;
			std::shared_ptr<state> parent;
			index_owner(const size_type indx, std::shared_ptr<state> parent);
			~index_owner();
		};

		struct resource{
			std::unique_ptr<T> t;
			std::shared_ptr<state> parent;
			const size_type index;
			resource(std::unique_ptr<T> t, std::shared_ptr<state> parent, index_owner indx);
			~resource();
		};
		
		class LockedResource{
			const std::shared_ptr<const index_owner> index_preference;
			std::shared_ptr<state> parent;
			std::shared_ptr<resource> resource;

			//for when this resource is unmanaged due to overfull pull
			std::shared_ptr<std::unique_ptr<T> > single_resource;
			
		public:
			LockedResource(const LockedResource&) = delete;
			LockedResource(std::unique_ptr<T> t, std::shared_ptr<state> parent, index_owner indx);
			LockedResource(std::unique_ptr<T> t);

			T const * const operator->() const;
			T* operator->();

			T& operator*();
			const T& operator&() const;
			
			explicit LockedResource(const WeakResource& wr);

		};
		struct WeakResource{
			const std::shared_ptr<const index_owner> index_preference;
			std::shared_ptr<state> parent;
			std::weak_ptr<resource> resource;
			
			//for when this resource is unmanaged due to overfull pull
			std::weak_ptr<std::unique_ptr<T> > single_resource;
		public:
			LockedResource lock(Args && ... a);
			WeakResource(const WeakResource&) = delete;
			explicit WeakResource(const LockedResource& lr);
		};
		
		LockedResource acquire(Args && ... a);
	};
}

#include "resource_pool_impl.hpp"
