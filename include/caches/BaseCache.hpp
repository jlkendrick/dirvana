#ifndef BASECACHE_H
#define BASECACHE_H

#include "Types.h"
#include "FrequencyBased.hpp"
#include "RecentlyAccessed.hpp"
#include "ICache.h"
#include "nlohmann/json.hpp"

#include <list>
#include <string>
#include <iostream>
#include <unordered_map>

using ordered_json = nlohmann::ordered_json;


// Base cache template implementation, key is always a path (string) so we only need to specify the type of entry and the promotion policy
template <typename T, typename PromotionPolicy>
class BaseCache : public ICache, public PromotionPolicy {
public:
	BaseCache() = default;
	
	// Implementation that is only used for testing and when building the cache (so we can pass strings instead of json objects)
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

	void add(const ordered_json& entry) override {
		std::string path = entry["path"].get<std::string>();

		if (contains(path))
			return;
		
		T new_entry = this->create_entry(entry);

		order.push_back(new_entry);
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
	
	// Note: we have to set default value here as well as in the function signature so polymorphism can work correctly
	std::vector<std::string> get_all_paths(int max_results = -1) const override {
		int count = 0;
		std::vector<std::string> paths;
		for (const auto& entry : order) {
			// If max_results is set and we've reached the limit, break out of the loop
			if (max_results > 0 && count >= max_results)
				break;
			paths.push_back(this->get_path(entry));
			count++;
		}

		return paths;
	}

	ordered_json serialize_entries() const override {
		ordered_json serialized_entries = ordered_json::array();
		for (const auto& entry : order) {
			serialized_entries.push_back(entry.serialize());
		}
		return serialized_entries;
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

// Define the concrete cache types
using RecentlyAccessedCache = BaseCache<RACEntry, RecentlyAccessedPolicy>;
using FrequencyBasedCache = BaseCache<FBCEntry, FrequencyBasedPolicy>;
	
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