#include "DirectoryCompleter.h"


// Only used for testing and when rebuilding the cache (so we can pass strings instead of json objects)
void DirectoryCompleter::add(const std::string& path, const std::string& dirname) {
	// Get the deepest directory name in the path which is the key for the cache
	auto res = dirname.empty() ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;
	
	// Add the path to the cache or do nothing if the path is already in the cache, create a new cache if needed
	if (path_map.find(res.second) == path_map.end())
		path_map[res.second] = CacheFactory::create_cache(strategy);
	path_map[res.second]->add(path);
}

void DirectoryCompleter::add(const ordered_json& entry, const std::string& dirname) {
	// Get the deepest directory name in the path which is the key for the cache
	auto res = dirname.empty() ? get_deepest_dir(entry["path"]) : std::make_pair(true, dirname);
	if (!res.first)
		return;
	
	// Add the path to the cache or do nothing if the path is already in the cache, create a new cache if needed
	if (path_map.find(res.second) == path_map.end())
		path_map[res.second] = CacheFactory::create_cache(strategy);
	path_map[res.second]->add(entry);
}

void DirectoryCompleter::remove(const std::string& path, const std::string& dirname) {
	// If the dirname was not passed (when called from tests), get it from the path
	auto res = dirname.empty() ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;

	// Remove the path from the cache
	path_map[res.second]->remove(path);
}

void DirectoryCompleter::access(const std::string& path, const std::string& dirname) {
	// If the dirname was not passed (when called from tests), get it from the path
	auto res = dirname.empty() ? get_deepest_dir(path) : std::make_pair(true, dirname);
	if (!res.first)
		return;

	// If the dirname does not have a cache for it, create one
	if (path_map.find(res.second) == path_map.end())
		path_map[res.second] = CacheFactory::create_cache(strategy);

	// Access the path in the cache
	path_map[res.second]->access(path);
}

std::vector<std::string> DirectoryCompleter::get_matches(const std::string& query) const {
	// If no directory is passed, return all paths in the map
	if (query.empty()) {
		std::vector<std::string> all_paths;
		for (const auto& entry : path_map)
			for (const auto& path : entry.second->get_all_paths())
				all_paths.push_back(path);
		return all_paths;
	}

	// Get the matching type and max results from the config
	MatchingType type = TypeConversions::s_to_matching_type(config["matching"]["type"].get<std::string>());
	int max_results = config["matching"]["max_results"].get<int>();

	// Lambda to handle the different types of matching
	auto handle_matching = [&](MatchingType match_type) -> std::vector<std::string> {

		// Go through all entries the map and check if the key starts with the query to find matching directories
		std::vector<std::string> matching_dirs;
		for (const auto& [key, _] : path_map) {
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
			all_matches.push_back(path_map.at(dir)->get_all_paths());

		// Merge the locally sorted vectors into a single globally sorted vector based on the matching type
		// We use the history of accesses to prioritize paths across the different caches for recently-accessed
		// We can juse use the access_count for frequency-based promotion because it captures cross-cache accesses
		return merge_k_sorted_lists(all_matches, max_results);
	};

	// Handle the different types of matching
	if (type == MatchingType::Exact) {
		// If the directory is in the map, get the paths from the cache
		if (path_map.find(query) != path_map.end())
			// Return the first max_results paths from the cache for the exact match
			return path_map.at(query)->get_all_paths(max_results);
	} else if (type == MatchingType::Prefix || type == MatchingType::Suffix || type == MatchingType::Contains) {
		return handle_matching(type);
	}

	return {};
}

std::unordered_map<std::string, int> DirectoryCompleter::access_history; // Initialize the static access history map
bool DirectoryCompleter::AHComparator::operator()(const std::tuple<int, int, std::string>& a, const std::tuple<int, int, std::string>& b) {
	// Get the paths from the tuples
	std::string path_a = std::get<2>(a); // The path is the third element in the tuple
	std::string path_b = std::get<2>(b);
	
	// Compare the access history counts for the two paths (if they exist in the access history)
	int count_a = access_history.find(path_a) != access_history.end() ? access_history[path_a] : 0; // Default to 0 if not found
	int count_b = access_history.find(path_b) != access_history.end() ? access_history[path_b] : 0;

	return count_a < count_b; // Lower counts should come first
};

std::vector<std::string> DirectoryCompleter::merge_k_sorted_lists(const std::vector<std::vector<std::string>>& lists, int max_results) const {
	std::vector<std::string> results;

	// Create a min-heap with (index of vector path is in, index of the path in the vector, path itself)
	std::priority_queue<std::tuple<int, int, std::string>, std::vector<std::tuple<int, int, std::string>>, AHComparator> min_heap;
	// Initialize the min-heap with the first element from each list
	for (int i = 0; i < lists.size(); ++i) {
		if (lists[i].empty()) continue; // Skip empty lists
		min_heap.push(std::make_tuple(i, 0, lists[i][0]));
	}

	while (!min_heap.empty() && (results.size() < max_results)) {
		// Get the smallest element from the heap
		auto [list_index, element_index, path] = min_heap.top();
		min_heap.pop();

		// Add the path to the results
		results.push_back(path);

		// If there is a next element in the same list, push it to the heap
		if (element_index + 1 < lists[list_index].size()) {
			min_heap.push(std::make_tuple(list_index, element_index + 1, lists[list_index][element_index + 1]));
		}
	}

	return results;
};


int DirectoryCompleter::get_size() const {
	int size = 0;
	for (auto& entry : path_map)
		size += entry.second->get_size();
	return size;
}

// Gets all the keys (directory names) in the map so we can use them for prefix/suffix matching
std::vector<std::string> DirectoryCompleter::get_all_keys() const {
	std::vector<std::string> keys;
	for (const auto& entry : path_map)
		keys.push_back(entry.first);
	return keys;
}