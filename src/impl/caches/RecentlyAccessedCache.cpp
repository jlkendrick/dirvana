#include "RecentlyAccessedCache.h"

void RecentlyAccessedCache::promote(const std::string& path) {
	auto it = cache[path];
	RACEntry entry = *it;
		
	// Move the entry to the front of the list
	order.erase(it);
	order.push_front(entry);
	cache[path] = order.begin();
}