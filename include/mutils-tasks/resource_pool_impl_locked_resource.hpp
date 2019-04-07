#pragma once

namespace mutils {
	namespace resource_pool {	
	template<typename T, typename... Args>
	LockedResource<T,Args...>::LockedResource(std::shared_ptr<const index_owner> indx, std::shared_ptr<state> parent, std::shared_ptr<rented_resource> rsource)
		:index_preference(indx),
		 parent(parent),
		 rsource(rsource){
		assert(rsource);
	}
	
	template<typename T, typename... Args>
	LockedResource<T,Args...>::LockedResource(LockedResource&& o)
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
	const T * LockedResource<T,Args...>::operator->() const {
		return rsource->t.get();
	}
	
	template<typename T, typename... Args>
	T* LockedResource<T,Args...>::operator->() {
		return rsource->t.get();
	}
	
	template<typename T, typename... Args>
	T& LockedResource<T,Args...>::operator*() {
		return *rsource->t;
	}
	
	template<typename T, typename... Args>
	const T& LockedResource<T,Args...>::operator&() const {
		return *rsource->t;
	}

	template<typename T, typename... Args>
	LockedResource<T,Args...>
	LockedResource<T,Args...>::lock(const Args& ...){
		return acquire_if_locked();
	}

	template<typename T, typename... Args>
	LockedResource<T,Args...>
	LockedResource<T,Args...>::acquire_if_locked() const {
		return LockedResource{WeakResource<T,Args...>{*this}};
	}

	template<typename T, typename... Args>
	bool LockedResource<T,Args...>::is_locked() const {
		return true;
	}
	
	template<typename T, typename... Args>
	std::pair<std::size_t,resource_type> LockedResource<T,Args...>:: which__resource_type() const {
		return rsource->which__resource_type();
	}

	template<typename T, typename... Args>
	LockedResource<T,Args...> LockedResource<T,Args...>::clone(){
		return LockedResource(*this);
	}

	template<typename T, typename... Args>
	WeakResource<T,Args...> LockedResource<T,Args...>::weak(){
		return WeakResource<T,Args...>(*this);
	}
	
	template<typename T, typename... Args>
	LockedResource<T,Args...>::LockedResource(const LockedResource& o)
		:index_preference(o.index_preference),
		 parent(o.parent),
		 rsource(o.rsource)
	{}
	
	template<typename T, typename... Args>
	LockedResource<T,Args...>::LockedResource(const WeakResource<T,Args...>& wr)
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
}
