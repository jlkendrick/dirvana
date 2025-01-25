#include "FrequencyBasedCache.h"

#include "Node.h"

#include <memory>
#include <iostream>
#include <string>
#include <vector>

void FrequencyBasedCache::add(const std::string& path) {
	// If the path is already in the cache, increment its counter and update position
	if (this->contains(path)) {
		cache.at(path).access_count++;
		update_position(path);

		return;
	}

	// Create a new node and add it to the cache and the back of the list
	std::shared_ptr<Node> node = std::make_shared<Node>(path);
	cache.emplace(path, CacheEntry(node)); // Path is guaranteed to be unique here
	order.insert_back(node);

	size++;
}

void FrequencyBasedCache::update_position(const std::string& path) {
	CacheEntry& entry = cache.at(path);
	int current_count = entry.access_count;
	std::shared_ptr<Node> node_to_move = entry.node;

	// Remove the node from its current position
	order.delete_at(entry.node);

	std::shared_ptr<Node> current = order.get_dummy_head()->next;
	std::shared_ptr<Node> tail = order.get_dummy_tail();

	// Find the correct position based on frequency count
	while (current != tail) {
		// If we find a node with lower or equal frequency, insert before it
		if (cache.at(current->path).access_count <= current_count) {
			order.insert_before(node_to_move, current);
			return;
		}
		current = current->next;
	}

	// If we reach here, this node has the lowest frequency, insert at the back
	order.insert_back(node_to_move);
}

std::vector<std::string> FrequencyBasedCache::get_all_paths() const {
	return order.get_all_paths();
}