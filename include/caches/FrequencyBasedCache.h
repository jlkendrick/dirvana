#ifndef FREQUENCYBASEDCACHE_H
#define FREQUENCYBASEDCACHE_H

#include "BaseCache.h"
#include "utils/CacheEntry.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

// FrequencyBasedCache is a cache of paths ordered by their access frequency.
// It uses a map from paths to CacheEntries for quick access to the path's position in the cache,
// and maintains a counter for each path to track access frequency.
// The prioritization is done by reordering paths in a DoublyLinkedList based on access count.
class FrequencyBasedCache : public BaseCache<CacheEntry> {
public:
	void add(const std::string& entry) {
		BaseCache<CacheEntry>::add(entry, CacheEntry(entry));
	}

	// Updates the position of the path based on its access frequency
	void promote(const std::string& path) {
		// Get the iterator from the cache
		auto it = cache[path];
		
		// Increment the access count, remove the entry from its current position, and insert it at the new position
		CacheEntry updated_entry = *it;
		updated_entry.access_count++;
		
		order.erase(it);

		auto insert_pos = order.begin();
		while (insert_pos != order.end() && insert_pos->access_count > updated_entry.access_count)
			insert_pos++;
		auto new_it = order.insert(insert_pos, updated_entry);

		// Update the cache with the new iterator
		cache[path] = new_it;
	};

	std::vector<std::string> get_all_entries() const {
		std::vector<std::string> paths;
		for (const auto& entry : order)
			paths.push_back(entry.path);
		return paths;
	}
};

#endif // FREQUENCYBASEDCACHE_H