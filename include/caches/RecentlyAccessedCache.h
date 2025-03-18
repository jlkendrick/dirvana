#ifndef RECENTLYACCESSEDCACHE_H
#define RECENTLYACCESSEDCACHE_H

#include "BaseCache.h"

#include <list>
#include <memory>
#include <string>
#include <unordered_map>

// ------------------------------------------- SELF-IMPLEMENTED RECENTLY ACCESSED CACHE -------------------------------------------
// RecentlyAccessedCache is a cache of recently accessed paths.
// It uses a map from paths to Nodes for quick access to the path's position in the cache.
// The prioritization is done by reordering the nodes in a doubly linked list.
// class RecentlyAccessedCacheV2 : public BaseCache<std::string> {
// public:
// 	void promote(const std::string& path) {
// 		// Get the iterator from the cache
// 		auto it = cache.at(path);
	
// 		// Remove the node from its current position and insert it at the front
// 		std::string node = *it;
// 		order.erase(it);
// 		order.push_front(node);
// 		cache[path] = order.begin();
// 	};
// };

#endif // RECENTLYACCESSEDCACHE_H