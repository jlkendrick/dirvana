#include "RecentlyAccessedCache.h"

#include "Node.h"

#include <memory>
#include <iostream>

void RecentlyAccessedCache::add(const std::string& path) {
	if (cache.find(path) != cache.end())
		return; // For now

	std::shared_ptr<Node> node = std::make_shared<Node>(path);
	cache[path] = node;
	order.insert_back(node);
	
	size++;
}

void RecentlyAccessedCache::promote(const std::string& path) {
	if (cache.find(path) == cache.end())
		return; // For now

	std::shared_ptr<Node> node = cache[path];
	order.delete_at(node);
	order.insert_front(node);
}