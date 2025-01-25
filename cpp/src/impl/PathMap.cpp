#include "PathMap.h"

#include "RecentlyAccessedCache.h"

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

	// std::cout << "Added " << path << " to cache for " << dir << " with size " << map[dir].get_size() << std::endl;

	size++;
}