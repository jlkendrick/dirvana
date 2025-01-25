#ifndef RECENTLYACCESSEDCACHE_H
#define RECENTLYACCESSEDCACHE_H

#include "Node.h"
#include "DoublyLinkedList.h"

#include <memory>
#include <string>
#include <unordered_map>

class RecentlyAccessedCache {
public:

	// Constructor
	// RecentlyAccessedCache();

	// Getters
	int get_size() const { return size; }

	void add(const std::string& path);
	void promote(const std::string& path);

private:
	std::unordered_map<std::string, std::shared_ptr<Node>> cache;
	DoublyLinkedList order;

	int size = 0;
};

#endif // RECENTLYACCESSEDCACHE_H