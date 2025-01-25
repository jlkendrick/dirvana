#include "RecentlyAccessedCache.h"

#include "Node.h"

#include <memory>
#include <iostream>
#include <string>
#include <vector>

void RecentlyAccessedCache::add(const std::string& path) {
	// If the path is already in the cache, that means it has been accessed again
	if (this->contains(path)) {
		promote(path);
		return;
	}

	// Create a new node and add it to the cache and the back of the list
	std::shared_ptr<Node> node = std::make_shared<Node>(path);
	cache.insert({path, node}); // Path is guaranteed to be unique here
	order.insert_back(node);
	
	size++;
}

void RecentlyAccessedCache::promote(const std::string& path) {
	// Get the node from the cache
	std::shared_ptr<Node> node = cache.at(path);

	// Remove the node from its current position and insert it at the front
	order.delete_at(node);
	order.insert_front(node);
}

std::vector<std::string> RecentlyAccessedCache::get_paths() const {
	return order.get_paths();
}