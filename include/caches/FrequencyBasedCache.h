#ifndef FREQUENCYBASEDCACHE_H
#define FREQUENCYBASEDCACHE_H

#include "BaseCache.h"

#include <list>
#include <memory>
#include <string>
#include <unordered_map>

// FrequencyBasedCache is a cache of paths ordered by their access frequency.
// It uses a map from paths to CacheEntries for quick access to the path's position in the cache,
// and maintains a counter for each path to track access frequency.
// The prioritization is done by reordering paths in a DoublyLinkedList based on access count.
class FrequencyBasedCache : public BaseCache<std::string> {
public:

	// Updates the position of the path based on its access frequency
	// Called after incrementing the access counter
	void promote(const std::string& path) {
		// Get the iterator from the cache
		auto it = cache.at(path);
		int current_count = it->access_count;
	
		// Remove the path from its current position and insert it in the correct position
		order.erase(it);
		
		// Find the correct position based on frequency count
		auto current = order.begin();
		auto tail = order.end();
		while (current != tail) {
			// If we find a path with lower or equal frequency, insert before it
			if (current->access_count <= current_count) {
				order.insert(current, *it);
				return;
			}
			current++;
		}
	};

	std::vector<std::string> get_all_entries() const {
		std::vector<std::string> paths;
		for (const auto& entry : order)
			paths.push_back(*entry.path);
		return paths;
	}

private:
	// Struct to hold both the path and its access count
	struct CacheEntry {
		std::shared_ptr<std::string> path;
		int access_count;

		CacheEntry(std::shared_ptr<std::string> n) : path(n), access_count(1) {}
	};

	std::unordered_map<std::string, std::list<CacheEntry>::iterator> cache;
	std::list<CacheEntry> order;

	// Total number of paths in the cache
	int size = 0;
};

#endif // FREQUENCYBASEDCACHE_H