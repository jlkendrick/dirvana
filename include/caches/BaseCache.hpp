#ifndef BASECACHE_H
#define BASECACHE_H

#include "Types.h"
#include "ICache.h"
#include <json.hpp>

#include <list>
#include <string>
#include <unordered_map>

using ordered_json = nlohmann::ordered_json;


// Base cache template implementation, key is always a path (string) so we only need to specify the type of entry and the promotion policy
template <typename T>
class BaseCache : public ICache {
public:
	BaseCache() = default;
	
	// Implementation that is only used for testing and when building the cache (so we can pass strings instead of json objects)
	void add(const std::string& path) override {
		// If the path is already in the cache, do nothing
		if (contains(path))
			return;
				
		// Create the appropriate entry
		T entry = create_entry(path);
		
		// Add the entry to the list
		order.push_back(entry);
		cache[path] = --order.end();
		size++;
	}

	void add(const ordered_json& entry) override {
		std::string path = entry["path"].get<std::string>();

		if (contains(path))
			return;
		
		T new_entry = create_entry(entry);

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
			promote(path);

		// If we try to promote a path that is not in the cache, add it
		else
			add(path);
	}

	std::vector<std::string> get_paths(unsigned int max_results) const override {
		unsigned int count = 0;
		std::vector<std::string> paths;
		for (const auto& entry : order) {
			// If max_results is set and we've reached the limit, break out of the loop
			if (count++ >= max_results)
				break;
			paths.push_back(get_path(entry));
		}
	
		return paths;
	}

	std::vector<std::string> get_all_paths() const override {
		std::vector<std::string> paths;
		for (const auto& entry : order)
			paths.push_back(get_path(entry));
		return paths;
	}

	ordered_json serialize_entries(int max_results = -1) const override {
		int count = 0;
		ordered_json serialized_entries = ordered_json::array();
		for (const auto& entry : order) {
			// If max_results is set and we've reached the limit, break out of the loop
			if (max_results > 0 && count >= max_results)
				break;
			serialized_entries.push_back(entry.serialize());
			count++;
		}
		return serialized_entries;
	}
	
	bool contains(const std::string& path) const override {
		return cache.find(path) != cache.end();
	}
	
	int get_size() const override {
		return size;
	}
    
protected:
	std::unordered_map<std::string, typename std::list<T>::iterator> cache;
	std::list<T> order;
	int size = 0;

	// Pure virtual functions to be implemented by derived classes
	virtual T create_entry(const std::string& path) = 0;
	virtual T create_entry(const ordered_json& entry) = 0;
	virtual std::string get_path(const T& entry) const = 0;
	virtual void promote(const std::string& path) = 0;
};

#endif // BASECACHE_H