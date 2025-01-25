#include "RecentlyAccessedCache.h"

#include "Node.h"

void RecentlyAccessedCache::add(const std::string& path) {
	if (cache.find(path) != cache.end())
		return; // For now

	Node* node = new Node(path);
	cache[path] = node;
	order.insert_back(node);
}

void RecentlyAccessedCache::promote(const std::string& path) {
	if (cache.find(path) == cache.end())
		return; // For now

	Node* node = cache[path];
	order.delete_at(node);
	order.insert_front(node);
}