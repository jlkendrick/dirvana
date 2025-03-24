#ifndef BASECACHE_H
#define BASECACHE_H

#include "Types.h"

#include <list>
#include <string>
#include <iostream>
#include <unordered_map>

class ICache {
public:
	virtual ~ICache() = default;
	virtual void add(const std::string& path) = 0;
	virtual void remove(const std::string& path) = 0;
	virtual void access(const std::string& path) = 0;
	virtual std::vector<std::string> get_all_paths() const = 0;
	virtual bool contains(const std::string& path) const = 0;
	virtual int get_size() const = 0;
};

// Base cache template implementation, key is always a path (string) so we only need to specify the type of entry and the promotion policy
template <typename T, typename PromotionPolicy>
class BaseCache : public ICache, public PromotionPolicy {
public:
	BaseCache() = default;
	
	void add(const std::string& path) override {
		// If the path is already in the cache, do nothing
		if (contains(path))
			return;
				
		// Create the appropriate entry
		T entry = this->create_entry(path);
		
		// Add the entry to the list
		order.push_back(entry);
		cache[path] = --order.end();
		size++;
	}
	
	void remove(const std::string& path) override {
		// If the path is not in the cache, do nothing
		if (!contains(path))
			return;
				
		auto it = cache[path];
		order.erase(it);
		cache.erase(path);
		size--;
	}
	
	void access(const std::string& path) override {
		// Promote the path in the cache
		if (contains(path))
			this->template promote<BaseCache<T, PromotionPolicy>>(path);

		// If we try to promote a path that is not in the cache, add it
		else
			add(path);
	}
	
	std::vector<std::string> get_all_paths() const override {
		std::vector<std::string> paths;
		for (const auto& entry : order) {
			paths.push_back(this->get_path(entry));
		}
		return paths;
	}
	
	bool contains(const std::string& path) const override {
		return cache.find(path) != cache.end();
	}
	
	int get_size() const override {
		return size;
	}
    
	std::unordered_map<std::string, typename std::list<T>::iterator> cache;
	std::list<T> order;
	int size = 0;
};

// Recently accessed promotion policy
class RecentlyAccessedPolicy {
public:
	struct RACEntry {
		std::string path;

		RACEntry() : path("") {}
		RACEntry(const std::string& p) : path(p) {}
	};

	RACEntry create_entry(const std::string& path) const {
		return RACEntry(path);
	}
			
	std::string get_path(const RACEntry& entry) const {
		return entry.path;
	}
			
	template <typename CacheType>
	void promote(const std::string& path) {
		CacheType* self = static_cast<CacheType*>(this);
		auto it = self->cache[path];
		RACEntry entry = *it;
			
		// Move the entry to the front of the list
		self->order.erase(it);
		self->order.push_front(entry);
		self->cache[path] = self->order.begin();
	}
};
	
// Frequency-based promotion policy
class FrequencyBasedPolicy {
public:
	struct FBCEntry {
		std::string path;
		int access_count;
		
		FBCEntry() : path(""), access_count(-1) {}
		FBCEntry(const std::string& p) : path(p), access_count(1) {}
	};

	FBCEntry create_entry(const std::string& path) const {
		return FBCEntry(path);
	}
		
	std::string get_path(const FBCEntry& entry) const {
		return entry.path;
	}
		
	template <typename CacheType>
	void promote(const std::string& path) {
		CacheType* self = static_cast<CacheType*>(this);
		auto it = self->cache[path];
		FBCEntry entry = *it;
			
		// Increment the access count
		entry.access_count++;
			
		// Remove the entry from its current position
		self->order.erase(it);
			
		// Find the position to insert based on access count (higher counts first)
		auto insert_pos = self->order.begin();
		while (insert_pos != self->order.end() && 
					 insert_pos->access_count > entry.access_count) {
			++insert_pos;
		}
			
		// Insert the entry at the appropriate position
		auto new_it = self->order.insert(insert_pos, entry);
		self->cache[path] = new_it;
	}
};
	
// Define the concrete cache types
using RecentlyAccessedCache = BaseCache<RecentlyAccessedPolicy::RACEntry, RecentlyAccessedPolicy>;
using FrequencyBasedCache = BaseCache<FrequencyBasedPolicy::FBCEntry, FrequencyBasedPolicy>;
	
// Factory for creating the appropriate cache type
class CacheFactory {
public:
	static std::unique_ptr<ICache> create_cache(PromotionStrategy strategy) {
		if (strategy == PromotionStrategy::FREQUENCY_BASED) {
			return std::make_unique<FrequencyBasedCache>();
		}
		return std::make_unique<RecentlyAccessedCache>();
	}
};

#endif // BASECACHE_H