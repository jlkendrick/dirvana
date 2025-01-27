#ifndef PATHMAP_H
#define PATHMAP_H

#include "RecentlyAccessedCache.h"

#include <string>
#include <unordered_map>
#include <vector>

// PathMap is the primary data structure that holds all the directories.
// It is a map of the deepest directory name to a cache of recently accessed paths
// which have that directory name.
struct PathMap {

	PathMap() = default;

	// Adds a new path to the cache containing other paths with the same directory name
	// Ex. Adding "/Users/jameskendrick/Code/Projects/dirvana/cpp/src" will add it to the cache for "src"
	// then adding "/Users/jameskendrick/Code/Projects/courtvision/src" will add it to that same cache
	// that way, users can simply type the directory name and get the most recently accessed paths
	void add(const std::string& path, const std::string& dirname = "");

	// Helper function to return the deepest directory name in a path
	// Ex. get_deepest_dir("/Users/jameskendrick/Code/Projects/dirvana/cpp/src") will return "src"
	// We return a pair of bool and string to ensure that the path is valid
	std::pair<bool, std::string> get_deepest_dir(const std::string& path) const;

	// Returns the paths in the cache for the given directory name
	std::vector<std::string> get_all_paths(const std::string& dir) const;

	// Map of directory names to caches of recently accessed paths
	std::unordered_map<std::string, RecentlyAccessedCache> map;

	// Total number of paths in the map
	int size = 0;
};

#endif // PATHMAP_H