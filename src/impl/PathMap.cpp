#include "DirectoryCompleter.h"
#include "Helpers.h"

#include <string>
#include <iostream>

using PathMap = DirectoryCompleter::PathMap;

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

std::vector<std::string> PathMap::get_matches(const std::string& dir, const MatchingType& type) const {
	// If no directory is passed, return all paths in the map
	if (dir.empty()) {
		std::vector<std::string> all_paths;
		for (const auto& entry : map)
			for (const auto& path : entry.second.get_all_paths())
				all_paths.push_back(path);
		return all_paths;
	}

	// Handle different types of matching
	switch (type) {
		case MatchingType::Exact:
			// This is the default case, we just look for the directory name in the map
			if (map.find(dir) != map.end())
				return map.at(dir).get_all_paths();
			break;

		case MatchingType::Prefix:
			// TODO: Implement prefix matching

		case MatchingType::Suffix:
			// TODO: Implement suffix matching

		default:
			return std::vector<std::string>();
	}
}

int PathMap::get_size() const {
	int size = 0;
	for (auto& entry : map)
		size += entry.second.get_size();
	return size;
}

// Gets all the keys (directory names) in the map so we can use them for prefix/suffix matching
std::vector<std::string> PathMap::get_all_keys() const {
	std::vector<std::string> keys;
	for (const auto& entry : map)
		keys.push_back(entry.first);
	return keys;
}