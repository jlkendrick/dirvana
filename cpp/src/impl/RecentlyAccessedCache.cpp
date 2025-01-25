#include "RecentlyAccessedCache.h"

#include "Node.h"

#include <memory>
#include <iostream>
#include <string>
#include <vector>

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

std::vector<std::string> RecentlyAccessedCache::get() const {
	std::vector<std::string> paths;
	
	std::shared_ptr<Node> current = order.get_dummy_head()->next;
	std::shared_ptr<Node> tail = order.get_dummy_tail();
	while (current != tail) {
		paths.push_back(current->path);
		current = current->next;
	}

	return paths;
}