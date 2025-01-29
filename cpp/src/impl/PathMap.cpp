#include "PathMap.h"

#include <string>
#include <iostream>

void PathMap::add(const std::string& path, const std::string& dirname) {
	// If the dirname was not passed (when called from tests), get it from the path
	auto res = dirname == "" ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;

	// Add the path to the cache, this will create a new cache if it doesn't exist
	map[res.second].add(path);

	size++;
}

const std::shared_ptr<DoublyLinkedList> PathMap::get_list_for(const std::string& dir) const {
	// If the query directory is not in the map, return an empty list
	if (map.find(dir) == map.end())
		return std::make_shared<DoublyLinkedList>();
	return std::make_shared<DoublyLinkedList>(map.at(dir).get_list());
}

std::vector<std::string> PathMap::get_all_paths(const std::string& dir) const {
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