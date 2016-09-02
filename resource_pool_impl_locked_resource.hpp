#pragma once

namespace mutils {
		
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(std::shared_ptr<const index_owner> indx, std::shared_ptr<state> parent, std::shared_ptr<rented_resource> rsource)
		:index_preference(indx),
		 parent(parent),
		 rsource(rsource){
		assert(rsource);
	}
	
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(LockedResource&& o)
		:index_preference(o.index_preference),
		 parent(o.parent),
		 rsource(o.rsource)
	{
		o.index_preference.reset();
		o.parent.reset();
		o.rsource.reset();
		assert(rsource);
	}

	template<typename T, typename... Args>
	T const * const ResourcePool<T,Args...>::LockedResource::operator->() const {
		return rsource->t.get();
	}
	
	template<typename T, typename... Args>
	T* ResourcePool<T,Args...>::LockedResource::operator->() {
		return rsource->t.get();
	}
	
	template<typename T, typename... Args>
	T& ResourcePool<T,Args...>::LockedResource::operator*() {
		return *rsource->t;
	}
	
	template<typename T, typename... Args>
	const T& ResourcePool<T,Args...>::LockedResource::operator&() const {
		return *rsource->t;
	}

	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource
	ResourcePool<T,Args...>::LockedResource::lock(const Args& ...){
		return acquire_if_locked();
	}

	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource
	ResourcePool<T,Args...>::LockedResource::acquire_if_locked() const {
		return LockedResource{WeakResource{*this}};
	}

	template<typename T, typename... Args>
	bool ResourcePool<T,Args...>::LockedResource::is_locked() const {
		return true;
	}
	
	template<typename T, typename... Args>
	std::pair<std::size_t,std::string> ResourcePool<T,Args...>::LockedResource:: which_resource_type() const {
		return rsource->which_resource_type();
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::LockedResource::clone(){
		return LockedResource(*this);
	}
	
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(const LockedResource& o)
		:index_preference(o.index_preference),
		 parent(o.parent),
		 rsource(o.rsource)
	{}
	
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(const WeakResource& wr)
		:index_preference(wr.index_preference),
		 parent(wr.parent),
		 rsource(wr.rsource.lock())
	{
		assert(rsource);
		assert(parent);
		assert(index_preference ? index_preference.use_count() > 1 : true);
		assert(index_preference ? wr.index_preference.use_count() > 1 : true);
	}

}
