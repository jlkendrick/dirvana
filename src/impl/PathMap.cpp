#include "DirectoryCompleter.h"

#include <string>
#include <iostream>

using PathMap = DirectoryCompleter::PathMap;

void PathMap::add(const std::string& path, const std::string& dirname) {
	// Get the deepest directory name in the path which is the key for the cache
	auto res = dirname.empty() ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;
	
	// Add the path to the cache or do nothing if the path is already in the cache, create a new cache if needed
	if (map.find(res.second) == map.end())
		map[res.second] = CacheFactory::create_cache(strategy);
	map[res.second]->add(path);
}

void PathMap::remove(const std::string& path, const std::string& dirname) {
	// If the dirname was not passed (when called from tests), get it from the path
	auto res = dirname.empty() ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;

	// Remove the path from the cache
	map[res.second]->remove(path);
}

void PathMap::access(const std::string& path, const std::string& dirname) {
	// If the dirname was not passed (when called from tests), get it from the path
	auto res = dirname.empty() ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;

	// If the dirname does not have a cache for it, create one
	if (map.find(res.second) == map.end())
		map[res.second] = CacheFactory::create_cache(strategy);

	// Access the path in the cache
	map[res.second]->access(path);
}

// const std::shared_ptr<DoublyLinkedList> PathMap::get_list_for(const std::string& dir) const {
// 	// If the query directory is not in the map, return an empty list
// 	if (map.find(dir) == map.end())
// 		return std::make_shared<DoublyLinkedList>();
// 	return std::make_shared<DoublyLinkedList>(map.at(dir).get_list());
// }

std::vector<std::string> PathMap::get_matches(const std::string& query, const MatchingType& type, int max_results) const {
	// If no directory is passed, return all paths in the map
	if (query.empty()) {
		std::vector<std::string> all_paths;
		for (const auto& entry : map)
			for (const auto& path : entry.second->get_all_paths())
				all_paths.push_back(path);
		return all_paths;
	}

	std::vector<std::string> matches; // Vector to hold the matches

	// Lambda to handle the different types of matching
	auto handle_matching = [&](MatchingType match_type) {

		// Go through all entries the map and check if the key starts with the query to find matching directories
		std::vector<std::string> matching_dirs;
		for (const auto& [key, _] : map) {
			bool is_match = false;

			switch (match_type) {
				case MatchingType::Prefix:
					is_match = (key.size() >= query.size() && key.find(query) == 0);
					break;

				case MatchingType::Suffix:
					is_match = (key.size() >= query.size() && key.rfind(query) == key.size() - query.size());
					break;

				case MatchingType::Contains:
					is_match = (key.find(query) != std::string::npos);
					break;

				default:
					break;
			}

			if (is_match)
				matching_dirs.push_back(key);
		}

		// For each matching directory, get the paths from the cache
		std::vector<std::vector<std::string>> all_matches;
		for (const auto& dir : matching_dirs) 
			all_matches.push_back(map.at(dir)->get_all_paths());

		// Go column-by-column to get the highest priority matches
		size_t max_depth = 0;
		for (const auto& vec : all_matches)
			max_depth = std::max(max_depth, vec.size());
		
		for (size_t i = 0; i < max_depth && matches.size() < max_results; i++) {
			for (const auto& vec : all_matches) {
				if (i < vec.size()) {
					matches.push_back(vec[i]);
					if (matches.size() >= max_results)
						break;
				}
			}
		}
	};

	// Handle the different types of matching
	if (type == MatchingType::Exact) {
		// If the directory is in the map, get the paths from the cache
		if (map.find(query) != map.end())
			matches = map.at(query)->get_all_paths();
	} else if (type == MatchingType::Prefix || type == MatchingType::Suffix || type == MatchingType::Contains) {
		handle_matching(type);
	}

	return matches;
}

int PathMap::get_size() const {
	int size = 0;
	for (auto& entry : map)
		size += entry.second->get_size();
	return size;
}

// Gets all the keys (directory names) in the map so we can use them for prefix/suffix matching
std::vector<std::string> PathMap::get_all_keys() const {
	std::vector<std::string> keys;
	for (const auto& entry : map)
		keys.push_back(entry.first);
	return keys;
}