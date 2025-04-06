#include "HistoryCache.h"

void HistoryCache::promote(const std::string& path) {
	auto it = cache[path];
	HCEntry entry = *it;
		
	// Move the entry to the front of the list
	order.erase(it);
	order.push_front(entry);
	cache[path] = order.begin();
}

int HistoryCache::lookup_index(const std::string& path) const {
	auto it = cache.find(path);
	if (it != cache.end()) {
		return it->second->index;
	}
	return current_index; // Return the max index (which is larger than any valid index) if not found since we are using a min-heap
}