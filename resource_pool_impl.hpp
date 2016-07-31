#pragma once

namespace mutils{

	template<typename T, typename... Args>
	ResourcePool::state::state(size_type max_resources, const decltype(builder) &builder)
		:resources(max_resources),
		 max_resources(max_resources),
		 builder(builder){}
	
	template<typename T, typename... Args>
	bool ResourcePool::state::pool_full() const {
		return max_resources == (current_max_index + 1)
			|| recycled_indices.size() == 0;
	}

	LockedResource ResourcePool::state::acquire_with_preference(size_type preference, Args && ...){
		if (preference > max_resources){
			//no preference!
		}
		else {
			//preference!
		}
	}

	template<typename T, typename... Args>
	ResourcePool::ResourcePool(size_type max_resources, const decltype(typename state::builder) &builder)
		:_state(new state(max_resources,builder)){}

	template<typename T, typename... Args>
	ResourcePool::index_owner::index_owner(const size_type indx, std::shared_ptr<state> parent)
				:indx(indx),parent(parent){}

	template<typename T, typename... Args>
	ResourcePool::index_owner::~index_owner(){
		parent->recycled_indices.add(indx);
	}

	template<typename T, typename... Args>
	ResourcePool::resource::resource(std::unique_ptr<T> t, std::shared_ptr<state> parent, index indx)
		:t(t),parent(parent){
		assert(!parent->resources.at(index).second);
	}

	template<typename T, typename... Args>
	ResourcePool::resource::~resource(){
		parent->resources.at(index).second = std::move(t);
	}
	
	template<typename T, typename... Args>
	ResourcePool::LockedResource::LockedResource(std::unique_ptr<T> t, std::shared_ptr<state> parent, index indx)
		:index_preference(new index_owner(indx,parent)),
		 parent(parent),
		 resource(std::move(t),parent,indx)
	{}

	template<typename T, typename... Args>
	T const * const ResourcePool::LockedResource::operator->() const {
		return resource->t.get();
	}
	
	template<typename T, typename... Args>
	T* ResourcePool::LockedResource::operator->() {
		return resource->t.get();
	}
	
	template<typename T, typename... Args>
	T& ResourcePool::LockedResource::operator*() {
		return *resources->t;
	}
	
	template<typename T, typename... Args>
	const T& ResourcePool::LockedResource::operator&() const {
		return *resources->t;
	}
			
	
	template<typename T, typename... Args>
	explicit ResourcePool::LockedResource::LockedResource(const WeakResource& wr)
		:index_preference(wr.index_preference),
		 parent(wr.parent),
		 resource(wr.resource.lock()){
		assert(resource);
	}

	template<typename T, typename... Args>
	LockedResource ResourcePool::WeakResource::lock(Args && ... a){
		auto locked = resource.lock();
		if (locked && *locked) 
			return LockedResource(*this);
		else {
			return parent->acquire_with_preference(index_preference->indx,std::forward<Args>(a)...);
		}
	}

	template<typename T, typename... Args>
	explicit ResourcePool::WeakResource::WeakResource(const LockedResource& lr)
		:index_preference(lr.index_preference),
		 parent(lr.parent),
		 resource(lr.resource)
	{}

	template<typename T, typename... Args>
	LockedResource ResourcePool::acquire_with_preference(size_type preference, Args && ... a){
		return _state->acquire_with_preference(preference,std::forward<Args>(a)...);
	}

	template<typename T, typename... Args>
	LockedResource ResourcePool::acquire(Args && ... a){
		return _state->acquire_with_preference(max_resources + 1, std::forward<Args>(a)...);
	}
}
