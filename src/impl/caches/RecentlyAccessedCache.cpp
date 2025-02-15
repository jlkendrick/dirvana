#include "RecentlyAccessedCache.h"

#include "Node.h"

#include <memory>
#include <iostream>
#include <string>
#include <vector>

void RecentlyAccessedCache::add(const std::string& path) {
	// If the path is already in the cache, do nothing. This should only happen when 'refreshing' the cache, 
	// in which case we preserve the existing order and put new paths at the back
	if (this->contains(path))
		return;

	// Create a new node and add it to the cache and the back of the list
	std::shared_ptr<Node> node = std::make_shared<Node>(path);
	cache.insert({path, node}); // Path is guaranteed to be unique here
	order.insert_back(node);
	
	size++;
}

void RecentlyAccessedCache::access(const std::string& path) {
	// If the path is already in the cache, promote it
	if (this->contains(path)) {
		promote(path);
		return;
	}

	// If the path is not in the cache, add it
	add(path);
}

void RecentlyAccessedCache::bulk_load(const std::vector<std::string>& paths) {
	// Clear existing cache and order list if needed
	cache.clear();
	order.clear();
	size = 0;
	
	// Add paths in the given order
	for (const auto& path : paths) {
		std::shared_ptr<Node> node = std::make_shared<Node>(path);
		cache.insert({path, node});
		order.insert_back(node);
		size++;
	}
}

void RecentlyAccessedCache::promote(const std::string& path) {
	// Get the node from the cache
	std::shared_ptr<Node> node = cache.at(path);

	// Remove the node from its current position and insert it at the front
	order.delete_at(node);
	order.insert_front(node);
}

std::vector<std::string> RecentlyAccessedCache::get_all_paths() const {
	return order.get_all_paths();
}