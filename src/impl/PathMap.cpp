#include "PathMap.h"

#include <string>
#include <iostream>

void PathMap::add(const std::string& path, const std::string& dirname) {
	// Get the deepest directory name in the path which is the key for the cache
	auto res = dirname.empty() ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;

	// Add the path to the cache or do nothing if the path is already in the cache, this will also create a new cache if needed
	map[res.second].add(path);
}

void PathMap::remove(const std::string& path, const std::string& dirname) {
	// If the dirname was not passed (when called from tests), get it from the path
	auto res = dirname.empty() ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;

	// Remove the path from the cache
	map[res.second].remove(path);
}

void PathMap::access(const std::string& path, const std::string& dirname) {
	// If the dirname was not passed (when called from tests), get it from the path
	auto res = dirname.empty() ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;

	// Access the path in the cache
	map[res.second].access(path);
}

// const std::shared_ptr<DoublyLinkedList> PathMap::get_list_for(const std::string& dir) const {
// 	// If the query directory is not in the map, return an empty list
// 	if (map.find(dir) == map.end())
// 		return std::make_shared<DoublyLinkedList>();
// 	return std::make_shared<DoublyLinkedList>(map.at(dir).get_list());
// }

std::vector<std::string> PathMap::get_all_paths(const std::string& dir) const {
	// If no directory is passed, return all paths in the map
	if (dir.empty()) {
		std::vector<std::string> all_paths;
		for (const auto& entry : map)
			for (const auto& path : entry.second.get_all_paths())
				all_paths.push_back(path);
		return all_paths;
	}

	// If the query directory is not in the map, return an empty vector
	if (map.find(dir) == map.end())
		return std::vector<std::string>();

	return map.at(dir).get_all_paths();
}

std::pair<bool, std::string> PathMap::get_deepest_dir(const std::string& path) const {
	size_t pos = path.find_last_of('/');
	if (pos >= std::string::npos)
		return std::make_pair(false, "");

	return std::make_pair(true, path.substr(pos + 1));
}

int PathMap::get_size() const {
	int size = 0;
	for (auto& entry : map)
		size += entry.second.get_size();
	return size;
}