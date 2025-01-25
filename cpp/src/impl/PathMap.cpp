#include "PathMap.h"

#include <string>
#include <iostream>

void PathMap::add(const std::string& path) {
	// Get the deepest directory name
	size_t pos = path.find_last_of('/');
	if (pos >= std::string::npos)
		return;
	std::string dir = path.substr(pos + 1);
	
	// Add the path to the cache, this will create a new cache if it doesn't exist
	map[dir].add(path);

	size++;
}

std::vector<std::string> PathMap::get_all_paths(const std::string& dir) const {
	// If the query directory is not in the map, return an empty vector
	if (map.find(dir) == map.end())
		return std::vector<std::string>();

	return map.at(dir).get_all_paths();
}