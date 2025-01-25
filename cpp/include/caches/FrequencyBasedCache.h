#ifndef FREQUENCYBASEDCACHE_H
#define FREQUENCYBASEDCACHE_H

#include "Node.h"
#include "DoublyLinkedList.h"

#include <memory>
#include <string>
#include <unordered_map>

// FrequencyBasedCache is a cache of paths ordered by their access frequency.
// It uses a map from paths to Nodes for quick access to the path's position in the cache,
// and maintains a counter for each path to track access frequency.
// The prioritization is done by reordering nodes in a DoublyLinkedList based on access count.
class FrequencyBasedCache {
public:
	// Constructs a new Node with the given path and adds it to the cache
	// If the path already exists, increments its access counter
	void add(const std::string& path);

	// Updates the position of the node based on its access frequency
	// Called after incrementing the access counter
	void update_position(const std::string& path);

	// Returns the paths in the cache ordered by access frequency
	std::vector<std::string> get_paths() const;
	
	// Returns true if the cache contains the given path, false otherwise
	bool contains(const std::string& path) const { return cache.find(path) != cache.end(); }

	// Returns the size of the cache
	int get_size() const { return size; }


private:
	// Struct to hold both the node and its access count
	struct CacheEntry {
		std::shared_ptr<Node> node;
		int access_count;

		CacheEntry(std::shared_ptr<Node> n) : node(n), access_count(1) {}
	};

	std::unordered_map<std::string, CacheEntry> cache;
	DoublyLinkedList order;

	// Total number of paths in the cache
	int size = 0;
};

#endif // FREQUENCYBASEDCACHE_H