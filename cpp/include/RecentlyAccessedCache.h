#ifndef RECENTLYACCESSEDCACHE_H
#define RECENTLYACCESSEDCACHE_H

#include "Node.h"
#include "DoublyLinkedList.h"

#include <unordered_map>

class RecentlyAccessedCache {
public:

	// Getters
	int get_size() const { return order.get_size(); }

	void add(const std::string& path);
	void promote(const std::string& path);

private:
	std::unordered_map<std::string, Node*> cache;
	DoublyLinkedList order;
};

#endif // RECENTLYACCESSEDCACHE_H