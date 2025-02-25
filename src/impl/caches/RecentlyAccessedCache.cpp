#include "RecentlyAccessedCache.h"

#include "Node.h"

#include <memory>
#include <iostream>
#include <string>
#include <vector>

// ------------------------------------------- SELF-IMPLEMENTED RECENTLY ACCESSED CACHE -------------------------------------------
void RecentlyAccessedCacheV1::add(const std::string& path) {
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

void RecentlyAccessedCacheV1::remove(const std::string& path) {
	// If the path is not in the cache, do nothing
	if (!this->contains(path))
		return;

	// Get the node from the cache
	std::shared_ptr<Node> node = cache.at(path);

	// Remove the node from the cache and the list
	cache.erase(path);
	order.delete_at(node);

	size--;
}

void RecentlyAccessedCacheV1::access(const std::string& path) {
	// If the path is already in the cache, promote it
	if (this->contains(path)) {
		promote(path);
		return;
	}

	// If the path is not in the cache, add it
	add(path);
}

void RecentlyAccessedCacheV1::promote(const std::string& path) {
	// Get the node from the cache
	std::shared_ptr<Node> node = cache.at(path);

	// Remove the node from its current position and insert it at the front
	order.delete_at(node);
	order.insert_front(node);
}

std::vector<std::string> RecentlyAccessedCacheV1::get_all_paths() const {
	return order.get_all_paths();
}

// ------------------------------------------- STD LIBRARY RECENTLY ACCESSED CACHE -------------------------------------------
void RecentlyAccessedCacheV2::add(const std::string& path) {
	// If the path is already in the cache, do nothing
	if (this->contains(path))
		return;

	// Create a new node and add it to the cache and the back of the list
	order.push_back(path);
	cache.insert({path, --order.end()}); // Path is guaranteed to be unique here

	size++;
}

void RecentlyAccessedCacheV2::remove(const std::string& path) {
	// If the path is not in the cache, do nothing
	if (!this->contains(path))
		return;

	// Get the iterator from the cache
	auto it = cache.at(path);

	// Remove the node from the cache and the list
	cache.erase(path);
	order.erase(it);

	size--;
}

void RecentlyAccessedCacheV2::access(const std::string& path) {
	// If the path is already in the cache, promote it
	if (this->contains(path)) {
		promote(path);
		return;
	}

	// If the path is not in the cache, add it
	add(path);
}

void RecentlyAccessedCacheV2::promote(const std::string& path) {
	// Get the iterator from the cache
	auto it = cache.at(path);

	// Remove the node from its current position and insert it at the front
	std::string node = *it;
	order.erase(it);
	order.push_front(node);
	cache[path] = order.begin();
}

std::vector<std::string> RecentlyAccessedCacheV2::get_all_paths() const {
	return std::vector<std::string>(order.begin(), order.end());
}