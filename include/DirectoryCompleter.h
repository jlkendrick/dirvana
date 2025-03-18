#ifndef DIRECTORYCOMPLETER_H
#define DIRECTORYCOMPLETER_H

#include "caches/BaseCache.h"
#include "nlohmann/json.hpp"
#include "utils/Helpers.h"
#include "utils/Types.h"

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <cstdlib>
#include <unordered_set>


using json = nlohmann::json;


// DirectoryCompleter is the main class that sets up the caches and is the API for the main program
class DirectoryCompleter {
public:

	// Constructors and destructor
	DirectoryCompleter(const DCArgs& args);
	DirectoryCompleter() = default;
	~DirectoryCompleter() = default;

	// Functions needed to get matches and update the cache ordering (proxies to PathMap so see PathMap for details)
	std::vector<std::string> get_matches(const std::string& dir = "") const {
		return directories.get_matches(
			dir, 
			s_to_matching_type(config["matching"]["type"].get<std::string>()), 
			config["matching"]["max_results"].get<int>());
	}
	void access(const std::string& path) { directories.access(path, get_deepest_dir(path).second); }
	
	// Members needed to manage exclusion rules for directory names
	void add_exclusion_rule(const ExclusionRule& rule) { exclusion_rules.push_back(rule); }
	
	// Functions to save and load the completer from a JSON file
	void save() const;
	void load(std::unordered_set<std::string>& old_dirs);
	
	// Utility functions (proxies to PathMap)
	int get_size() const { return directories.get_size(); }
	bool has_matches(const std::string& dir) const { return directories.contains(dir); }

	// Getters for the configuration data (used for testing)
	const json& get_config() const { return config; }

	// PathMap is the primary data structure that holds all the directories.
	// It is a map of the deepest directory name to a cache of recently accessed paths
	// which have that directory name.
	struct PathMap {

		PathMap(PromotionStrategy strat = PromotionStrategy::RECENTLY_ACCESSED) : strategy(strat) {};

		// Adds a new path to the cache containing other paths with the same directory name
		// Ex. Adding "/Users/jameskendrick/Code/Projects/dirvana/cpp/src" will add it to the cache for "src"
		// then adding "/Users/jameskendrick/Code/Projects/courtvision/src" will add it to that same cache
		// that way, users can simply type the directory name and get the most recently accessed paths
		void add(const std::string& path, const std::string& dirname = "");
		void remove(const std::string& path, const std::string& dirname = "");
		// Access the given path in the cache for the directory name
		void access(const std::string& path, const std::string& dirname = "");

		// Returns the paths in the cache for the given directory name
		std::vector<std::string> get_matches(const std::string& dir = "", const MatchingType& type = MatchingType::Exact, int max_results = 10) const;

		// Returns true if the map contains the given directory name
		bool contains(const std::string& dir) const { return map.find(dir) != map.end(); }
		// Returns the total number of paths in the map (sum of all caches)
		int get_size() const;
		// Returns all the paths in the map
		std::vector<std::string> get_all_keys() const;

		// Map of directory names to caches of recently accessed paths
		std::unordered_map<std::string, std::unique_ptr<ICache>> map;
		PromotionStrategy strategy = PromotionStrategy::RECENTLY_ACCESSED; // Promotion strategy for the caches, default is recently accessed
	};

private:
	PathMap directories; // PathMap object to hold the directories and caches

	json config; // JSON object to hold the config file
	std::string config_path = std::string(std::getenv("HOME")) + std::string("/Code/Projects/dirvana/config.json");
	json default_config = {
		{"paths", {
			{"init", std::getenv("HOME") + std::string("/")},
			{"cache", std::getenv("HOME") + std::string("/Code/Projects/dirvana/build/cache.json")}
		}},
		{"matching", {
			{"max_results", 10},
			{"type", "exact"},
			{"promotion_strategy", "recently_accessed"}
		}}
	}; // Default config file
	MatchingType s_to_matching_type(const std::string& type) const; // Converts the string to the MatchingType enum
	PromotionStrategy s_to_promotion_strategy(const std::string& type) const; // Converts the string to the PromotionStrategy enum
	json load_config() const; // Loads the config file from the given path to override the default settings if needed
	bool validate_config(json& user_config) const; // Validates and modifies the config file if needed, returns true if modified, false otherwise

	// Re-scans the root directory to add/remove directories
	void refresh_directories(std::unordered_set<std::string>& old_dirs);

	// Private functions to collect directories and check for exclusions when building the caches
	// Private function used by the constructor to collect all of the directories in the root directory
	std::vector<std::pair<std::string, std::string>> collect_directories();
	// Returns true if the given directory should be excluded from the PathMap
	bool should_exclude(const std::string& dir, const std::string& path = "") const;
	// Stores exclusion rules for directories
	std::vector<ExclusionRule> exclusion_rules = {
		// Exclude dot directories
		{ ExclusionType::Prefix, "." },
		// Exclude common auto-generated directories
		{ ExclusionType::Exact, "node_modules" },
		{ ExclusionType::Exact, "bower_components" },
		{ ExclusionType::Exact, "dist" },
		{ ExclusionType::Exact, "out" },
		{ ExclusionType::Exact, "target" },
		{ ExclusionType::Exact, "tmp" },
		{ ExclusionType::Exact, "temp" },
		{ ExclusionType::Exact, "cache" },
		{ ExclusionType::Exact, "venv" },
		{ ExclusionType::Exact, "env" },
		{ ExclusionType::Exact, "obj" },
		{ ExclusionType::Exact, "pkg" },
		{ ExclusionType::Exact, "bin" },
		// Suffix rules
		{ ExclusionType::Suffix, "sdk" },
		{ ExclusionType::Suffix, "Library" },
		// Contains rules
		{ ExclusionType::Contains, "release" }
	};
};

#endif // DIRECTORYCOMPLETER_H