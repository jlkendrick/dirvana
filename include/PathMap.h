#ifndef PATHMAP_H
#define PATHMAP_H

#include "RecentlyAccessedCache.h"

#include <memory>
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

	void remove(const std::string& path, const std::string& dirname = "");

	// Access the given path in the cache for the directory name
	void access(const std::string& path, const std::string& dirname = "");

	// Helper function to return the deepest directory name in a path
	// Ex. get_deepest_dir("/Users/jameskendrick/Code/Projects/dirvana/cpp/src") will return "src"
	// We return a pair of bool and string to ensure that the path is valid
	std::pair<bool, std::string> get_deepest_dir(const std::string& path) const;

	// Returns a reference to the DLL that is used to construct the traverser
	// const std::shared_ptr<DoublyLinkedList> get_list_for(const std::string& dir) const;

	// Returns the paths in the cache for the given directory name
	std::vector<std::string> get_all_paths(const std::string& dir = "") const;

	// Returns true if the map contains the given directory name
	bool contains(const std::string& dir) const { return map.find(dir) != map.end(); }

	// Returns the total number of paths in the map (sum of all caches)
	int get_size() const;

	// Map of directory names to caches of recently accessed paths
	// Here is where we can switch between the two implementations of V1 (self-implemented) and V2 (std::list)
	std::unordered_map<std::string, RecentlyAccessedCacheV2> map;

	// Total number of paths in the map
	int size = 0;
};

#endif // PATHMAP_H