#ifndef PATHMAP_H
#define PATHMAP_H

#include "RecentlyAccessedCache.h"

#include <string>
#include <unordered_map>

struct PathMap {

	// Constructor
	PathMap() = default;

	// Map of directory names to caches of recently accessed paths
	std::unordered_map<std::string, RecentlyAccessedCache> map;
	int size = 0;

	// Adds a new path to the cache containing other paths with the same directory name
	// Ex. Adding "/Users/jameskendrick/Code/Projects/dirvana/cpp/src" will add it to the cache for "src"
	// then adding "/Users/jameskendrick/Code/Projects/courtvision/src" will add it to that same cache
	// that way, users can simply type the directory name and get the most recently accessed paths
	void add(const std::string& path);
};

#endif // PATHMAP_H