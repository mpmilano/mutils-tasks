#pragma once

namespace mutils{

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::state::state(size_type max_resources, const decltype(builder) &builder)
		:max_resources(max_resources),
		 builder(builder){
		for (std::size_t i = 0; i < max_resources; ++i){
			resources.emplace_back(i);
		}
	}
	
	template<typename T, typename... Args>
	bool ResourcePool<T,Args...>::state::pool_full() const {
		return max_resources <= (current_max_index + 1)
			|| recycled_indices.size() == 0;
	}

	template<typename T, typename... Args>	
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::state::acquire_no_preference(std::shared_ptr<state> _this, Args && ... a){
		//don't even try to get a preference
		const static std::function<resource_pair* ()> nullfun = []()-> resource_pair* {return nullptr;};
		auto *cand = _this->free_resources.build_or_pop(nullfun);
		while (cand){
			lock l{cand->mut};
			if (cand->resource)
				return
					LockedResource{std::move(cand->resource),_this,index_owner{cand->index,_this}};
			else cand = _this->free_resources.build_or_pop(nullfun);
		}
		//whoops, entirely full!
		//just build a one-off resource
		return LockedResource{std::unique_ptr<T>{_this->builder(std::forward<Args>(a)...)}};
	}
	
	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::state::acquire_new_preference(std::shared_ptr<state> _this, Args && ... a){
		//no preference!
		if (!_this->pool_full()){
			//try to get a preference!
			auto my_preference = (_this->recycled_indices.size() > 0 ?
								  _this->recycled_indices.build_or_pop(
									  [&cmi = _this->current_max_index](){
										  return cmi.fetch_add(1);})
								  : _this->current_max_index.fetch_add(1));
			if (my_preference < _this->max_resources){
				//have a preference!
				return acquire_with_preference(_this,my_preference,std::forward<Args>(a)...);
			}
			else {
				//you lost, sorry. Do the no-preference-available route.
				return acquire_no_preference(_this,std::forward<Args>(a)...);
			}
		}
		else {
			return acquire_no_preference(_this,std::forward<Args>(a)...);
		}
	}

	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::state::acquire_with_preference(std::shared_ptr<state> _this, size_type preference, Args && ... a){
		//preference!
		auto &my_resource = _this->resources.at(preference);
		lock l{my_resource.mut};
		if (!my_resource.initialized){
			my_resource.initialized = true;
			my_resource.resource.reset(_this->builder(std::forward<Args>(a)...));
		}
		if (my_resource.resource){
			return LockedResource{std::move(my_resource.resource),
					_this,
					index_owner{preference,_this}};
		}
		else return acquire_no_preference(_this,std::forward<Args>(a)...);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::ResourcePool(size_type max_resources, const decltype(state::builder) &builder)
		:_state(new state(max_resources,builder)){}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::index_owner::index_owner(const size_type indx, std::shared_ptr<state> parent)
				:indx(indx),parent(parent){}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::index_owner::~index_owner(){
		assert(indx < parent->max_resources);
		parent->recycled_indices.add(indx);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::resource::resource(std::unique_ptr<T> t, std::shared_ptr<state> parent, index_owner index)
		:t(t),parent(parent){
		assert(!parent->resources.at(index).resource);
	}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::resource::~resource(){
		assert(indx < max_resources);
		lock l{parent->resources.at(index).mut};
		parent->resources.at(index).resource = std::move(t);
		parent->free_resources.add(&parent->resources.at(index));
	}
	
	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(std::unique_ptr<T> t, std::shared_ptr<state> parent, index_owner index)
		:index_preference(new index_owner(indx,parent)),
		 parent(parent),
		 resource(std::move(t),parent,indx),
		 single_resource(nullptr)
	{}

	template<typename T, typename... Args>
	ResourcePool<T,Args...>::LockedResource::LockedResource(std::unique_ptr<T> t)
		:index_preference(nullptr),
		 parent(nullptr),
		 resource(nullptr),
		 single_resource(new std::unique_ptr<T>{std::move(t)})
	{}

	template<typename T, typename... Args>
	T const * const ResourcePool<T,Args...>::LockedResource::operator->() const {
		return (resource ? resource->t.get() : single_resource->t.get());
	}
	
	template<typename T, typename... Args>
	T* ResourcePool<T,Args...>::LockedResource::operator->() {
		return (resource ? resource->t.get() : single_resource->t.get());
	}
	
	template<typename T, typename... Args>
	T& ResourcePool<T,Args...>::LockedResource::operator*() {
		return (resource ? *resources->t : *single_resource->t);
	}
	
	template<typename T, typename... Args>
	const T& ResourcePool<T,Args...>::LockedResource::operator&() const {
		return (resource ? *resources->t : *single_resource->t);
	}
			
	
	template<typename T, typename... Args>
	explicit ResourcePool<T,Args...>::LockedResource::LockedResource(const WeakResource& wr)
		:index_preference(wr.index_preference),
		 parent(wr.parent),
		 resource(wr.resource.lock()),
		 single_resource(wr.single_resource.lock())
	{
		assert(resource || single_resource);
	}

	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::WeakResource::lock(Args && ... a){
		auto locked = resource.lock();
		auto single_locked = single_resource.lock();
		if ((locked && *locked) || (single_locked && *single_locked))
			return LockedResource(*this);
		else if (index_preference){
			return parent->acquire_with_preference(parent,index_preference->indx,std::forward<Args>(a)...);
		}
		else {
			return parent->acquire_new_preference(parent,std::forward<Args>(a)...);
		}
	}

	template<typename T, typename... Args>
	explicit ResourcePool<T,Args...>::WeakResource::WeakResource(const LockedResource& lr)
		:index_preference(lr.index_preference),
		 parent(lr.parent),
		 resource(lr.resource),
		 single_resource(lr.single_resource)
	{}

	template<typename T, typename... Args>
	typename ResourcePool<T,Args...>::LockedResource ResourcePool<T,Args...>::acquire(Args && ... a){
		return _state->acquire_with_preference(_state,max_resources + 1, std::forward<Args>(a)...);
	}
}