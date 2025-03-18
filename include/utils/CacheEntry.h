#ifndef CACHEENTRY_H
#define CACHEENTRY_H

#include <string>

// Struct to hold both the path and its access count
struct CacheEntry {
	std::string path;
	int access_count;

	CacheEntry() : path(""), access_count(-1) {};
	CacheEntry(const std::string& n) : path(n), access_count(1) {}

	bool empty() const { return path.empty() && access_count == -1; }
};

#endif // CACHEENTRY_H