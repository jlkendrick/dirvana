#include "RecentlyAccessedCache.h"

#include "Node.h"

#include <memory>
#include <iostream>
#include <string>
#include <vector>

void RecentlyAccessedCache::add(const std::string& path) {
	// If the path is already in the cache, that means it has been accessed again
	if (cache.find(path) != cache.end()) {
		promote(path);
		return;
	}

	// Create a new node and add it to the cache and the back of the list
	std::shared_ptr<Node> node = std::make_shared<Node>(path);
	cache[path] = node;
	order.insert_back(node);
	
	size++;
}

void RecentlyAccessedCache::promote(const std::string& path) {
	// Get the node from the cache
	std::shared_ptr<Node> node = cache[path];

	// Remove the node from its current position and insert it at the front
	order.delete_at(node);
	order.insert_front(node);
}

std::vector<std::string> RecentlyAccessedCache::get() const {
	std::vector<std::string> paths;
	
	// Iterate through the list and add the paths to the vector
	std::shared_ptr<Node> current = order.get_dummy_head()->next;
	std::shared_ptr<Node> tail = order.get_dummy_tail();
	while (current != tail) {
		paths.push_back(current->path);
		current = current->next;
	}

	return paths;
}