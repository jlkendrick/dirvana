#ifndef DIRECTORYCOMPLETER_H
#define DIRECTORYCOMPLETER_H

#include "Helpers.h"
#include "HistoryCache.h"
#include <json.hpp>

#include <tuple>
#include <memory>
#include <string>
#include <vector>
#include <cstdlib>
#include <unordered_set>


// DirectoryCompleter is the main class that sets up the caches and is the API for the main program
class DirectoryCompleter {
public:
	// Constructors and destructor
	DirectoryCompleter(const DCArgs& args);
	DirectoryCompleter() = default;
	~DirectoryCompleter() = default;

	// PathMap methods
	void add(const std::string& path, const std::string& dirname = "");
	void add(const ordered_json& entry, const std::string& dirname = "");
	void remove(const std::string& path, const std::string& dirname = "");
	void access(const std::string& path, const std::string& dirname = "");
	std::vector<std::string> get_matches(const std::string& query = "") const;
		
	// Members needed to manage exclusion rules for directory names
	void add_exclusion_rule(const ExclusionRule& rule) { exclusion_rules.push_back(rule); }
	
	// Functions to save and load the completer from a JSON file
	void save() const;
	void load(std::unordered_set<std::string>& old_dirs);
	
	// Utility functions
	int get_size() const;
	bool contains(const std::string& dir) const { return path_map.find(dir) != path_map.end(); }
	std::vector<std::string> get_all_keys() const;

	// Getters for the configuration data (used for testing)
	const json& get_config() const { return config; }
	std::vector<std::string> get_history() const { return access_history.get_all_paths(); }
	void add_to_history(const std::string& path) { access_history.add(path); }

private:
	bool test_mode = false; // Flag to indicate if we are in test mode

	std::unordered_map<std::string, std::unique_ptr<ICache>> path_map; // Previously PathMap
	PromotionStrategy strategy = PromotionStrategy::RECENTLY_ACCESSED; // Promotion strategy for the caches, default is recently accessed

	// To quantify the history of accesses for the purposes of match return order prioritization, we map the accessed path to an int
	// The caches preseve the local order, but this serves as a recently_accessed layer on top of the caches that can be used to prioritize paths across all caches
	// Ex. cache1 = {A, B}, cache2 = {C, D}, history = {D:0, B:1, C:2, A:3}, output -> {C, D, A, B}
	HistoryCache access_history; // History cache to keep track of the access history
	struct AHComparator {
		const HistoryCache& history_ref;
		AHComparator(const HistoryCache& ah) : history_ref(ah) {}

		// Comparator for the priority queue to sort the paths based on the access history
		bool operator()(const std::tuple<int, int, std::string>& a, const std::tuple<int, int, std::string>& b);
	};

	json config; // JSON object to hold the config file
	std::string config_path = std::string(std::getenv("HOME")) + std::string("/.config/dirvana/config.json");
	json default_config = {
		{"paths", {
			{"init", std::getenv("HOME") + std::string("/")},
			{"cache", std::getenv("HOME") + std::string("/.cache/dirvana/")} // Just the prefix, the rest is added in the constructor based on the promotion strategy
		}},
		{"matching", {
			{"max_results", 10},
			{"max_history_size", 100},
			{"type", "exact"},
			{"promotion_strategy", "recently_accessed"},
			{"exclusions", {
				{"prefix", {"."}},
				{"exact", {"node_modules", "browser_components", "dist", "out", "target", "tmp", "temp", "cache", "venv", "env", "obj", "pkg", "bin"}},
				{"suffix", {"sdk", "Library"}},
				{"contains", {"release"}}
				}
			}
		}}
	}; // Default config file
	json load_config() const; // Loads the config file from the given path to override the default settings if needed
	std::vector<ExclusionRule> generate_exclusion_rules(const json& exclusions) const; // Generates the exclusion rules from the config file
	bool validate_config(json& user_config) const; // Validates and modifies the config file if needed, returns true if modified, false otherwise

	// Re-scans the root directory to add/remove directories
	void refresh_directories(std::unordered_set<std::string>& old_dirs);

	// Private functions to collect directories and check for exclusions when building the caches
	// Private function used by the constructor to collect all of the directories in the root directory
	std::vector<std::pair<std::string, std::string>> collect_directories();
	// Returns true if the given directory should be excluded from the PathMap
	bool should_exclude(const std::string& dir, const std::string& path = "") const;
	// Stores exclusion rules for directories
	std::vector<ExclusionRule> exclusion_rules;

	// Utils
	std::vector<std::string> merge_k_sorted_lists(const std::vector<std::vector<std::string>>& lists, unsigned int max_results) const; // Merges k sorted lists into a single sorted list based on the access history
};

#endif // DIRECTORYCOMPLETER_H