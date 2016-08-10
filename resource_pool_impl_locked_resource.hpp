#pragma once

namespace mutils {
		
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(std::unique_ptr<T> t, std::shared_ptr<state> parent, std::shared_ptr<const index_owner> indx)
		:index_preference(indx),
		 parent(parent),
		 rsource(new resource{std::move(t),parent,indx->indx}),
		 single_resource(nullptr),
		 spare_resource(nullptr)
	{}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(std::unique_ptr<T> t, std::shared_ptr<state> parent)
		:index_preference(nullptr),
		 parent(parent),
		 rsource(nullptr),
		 single_resource(new overdrawn_resource(parent,std::move(t))),
		 spare_resource(nullptr){}
	
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(std::unique_ptr<T> t, std::shared_ptr<state> parent, const size_type index)
		:index_preference(nullptr),
		 parent(parent),
		 rsource(nullptr),
		 single_resource(nullptr),
		 spare_resource(new struct spare_resource(parent,std::move(t),index)){}
	
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(LockedResource&& o)
		:index_preference(o.index_preference),
		 parent(o.parent),
		 rsource(o.rsource),
		 single_resource(o.single_resource),
		 spare_resource(o.spare_resource)
	{
		o.index_preference.reset();
		o.parent.reset();
		o.rsource.reset();
		o.single_resource.reset();
		o.spare_resource.reset();
	}

	template<typename T, typename... Args>
	T const * const ResourcePool<T,Args...>::LockedResource::operator->() const {
		return (rsource ? rsource->t.get() : (single_resource ? single_resource->t.get() : spare_resource->t.get()));
	}
	
	template<typename T, typename... Args>
	T* ResourcePool<T,Args...>::LockedResource::operator->() {
		return (rsource ? rsource->t.get() : (single_resource ? single_resource->t.get() : spare_resource->t.get()));
	}
	
	template<typename T, typename... Args>
	T& ResourcePool<T,Args...>::LockedResource::operator*() {
		return (rsource ? *rsource->t : (single_resource ? *single_resource->t : *spare_resource->t));
	}
	
	template<typename T, typename... Args>
	const T& ResourcePool<T,Args...>::LockedResource::operator&() const {
		return (rsource ? *rsource->t : (single_resource ? *single_resource->t : *spare_resource->t));
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
	ResourcePool<T,Args...>::LockedResource::LockedResource(const WeakResource& wr)
		:index_preference(wr.index_preference),
		 parent(wr.parent),
		 rsource(wr.rsource.lock()),
		 single_resource(wr.single_resource.lock()),
		 spare_resource(wr.spare_resource.lock())
	{
		assert(rsource || single_resource || spare_resource);
		assert((index_preference && parent && rsource)
			   || (parent && (single_resource || spare_resource)));
		assert(index_preference ? index_preference.use_count() > 1 : true);
		assert(index_preference ? wr.index_preference.use_count() > 1 : true);
	}

}
