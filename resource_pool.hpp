#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include "SafeSet.hpp"

namespace mutils{
	
	template<typename T, typename... Args>
	class ResourcePool{
		using resource_pair = std::pair<std::mutex, std::unique_ptr<T> >;
		std::vector<resource_pair> resources{max_resources};

		using size_type = typename decltype(resources)::size_type;
		
		const size_type max_resources;
		std::atomic<size_type> current_max_index{0};
		SafeSet<size_type> recycled_indices;

		const std::function<T* (Args...)> builder;
		
		bool pool_full() const {
			return max_resources == (current_max_index + 1) || recycled_indices.size() == 0;
		}
		
	public:

		struct index_owner{
			ResourcePool &parent;
			const int indx;
			~index_owner(){
				parent.recycled_indices.add(indx);
			}
		};

		struct resource{
			std::unique_ptr<T> t;
			ResourcePool &parent;
			const int index;
			~resource(){
				parent.resources.at(index).second = std::move(t);
			}
		};
		
		class LockedResource{
			const std::shared_ptr<const index_owner> index_preference;
			ResourcePool& parent;
			std::shared_ptr<resource> resource;
		public:
			LockedResource(const LockedResource&) = delete;
			explicit LockedResource(const WeakResource& wr)
				:index_preference(wr.index_preference),
				 resource(wr.resource.lock()){
				assert(resource);
			}
		};
		struct WeakResource{
			const std::shared_ptr<const index_owner> index_preference;
			ResourcePool& parent;
			std::weak_ptr<resource> resource;
		public:
			LockedResource lock(Args && ... a){
				auto locked = resource.lock();
				if (locked && *locked) 
					return LockedResource(*this);
				else {
					return parent.acquire_with_preference(index_preference->indx,std::forward<Args>(a)...);
				}
			}
			WeakResource(const WeakResource&) = delete;
		};

		LockedResource acquire_with_preference(size_type preference, Args &&& ...){
			if (preference > max_resources){
				//no preference!
			}
			else {
				//preference!
			}
		}
		
		LockedResource acquire(Args && ... a){
			return acquire_with_preference(max_resources + 1, std::forward<Args>(a)...);
		}
	};
}
