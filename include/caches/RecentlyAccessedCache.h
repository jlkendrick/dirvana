#ifndef RECENTLYACCESSEDCACHE_H
#define RECENTLYACCESSEDCACHE_H

#include "Node.h"
#include "DoublyLinkedList.h"

#include <memory>
#include <string>
#include <unordered_map>

// RecentlyAccessedCache is a cache of recently accessed paths.
// It uses a map from paths to Nodes for quick access to the path's position in the cache.
// The prioritization is done by reordering the nodes in a DoublyLinkedList.
class RecentlyAccessedCache {
public:

	// Constructs a new Node with the given path and adds it to the default position in the cache (back)
	void add(const std::string& path);

	// Removes the given path from the cache. If the path is not in the cache, nothing happens.
	void remove(const std::string& path);

	// Accesses the given path in the cache. If the path is already in the cache, it is promoted to the front.
	// If the path is not in the cache, it is added to the back.
	void access(const std::string& path);

	// Bulk loads a list of paths into the cache. This is used when loading the cache from a file.
	void bulk_load(const std::vector<std::string>& paths);

	// Promotes the given path to the front of the cache. Consists of removing the node from its current position
	// and inserting it at the front of the DoublyLinkedList.
	void promote(const std::string& path);

	// Returns a reference to the doubly linked list that can be used to construct the traverser
	const DoublyLinkedList& get_list() const { return order; }

	// Returns the paths in the cache in the order determined by the DoublyLinkedList
	std::vector<std::string> get_all_paths() const;

	// Returns true if the given path is in the cache, false otherwise
	bool contains(const std::string& path) const { return cache.find(path)!= cache.end(); }

	// Returns the size of the cache
	int get_size() const { return size; }

private:
	std::unordered_map<std::string, std::shared_ptr<Node>> cache;
	DoublyLinkedList order;

	// Total number of paths in the cache
	int size = 0;
};

#endif // RECENTLYACCESSEDCACHE_H