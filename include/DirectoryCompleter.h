#ifndef DIRECTORYCOMPLETER_H
#define DIRECTORYCOMPLETER_H

#include "caches/BaseCache.hpp"
#include "nlohmann/json.hpp"
#include "Helpers.h"
#include "Types.h"

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

	// PathMap methods
	void add(const std::string& path, const std::string& dirname = "");
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


private:
	bool test_mode = false; // Flag to indicate if we are in test mode

	std::unordered_map<std::string, std::unique_ptr<ICache>> path_map; // Previously PathMap
	PromotionStrategy strategy = PromotionStrategy::RECENTLY_ACCESSED; // Promotion strategy for the caches, default is recently accessed

	json config; // JSON object to hold the config file
	std::string config_path = std::string(std::getenv("HOME")) + std::string("/Code/Projects/dirvana/config.json");
	json default_config = {
		{"paths", {
			{"init", std::getenv("HOME") + std::string("/")},
			{"cache", std::getenv("HOME") + std::string("/.cache/dirvana/")} // Just the prefix, the rest is added in the constructor based on the promotion strategy
		}},
		{"matching", {
			{"max_results", 10},
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
};

#endif // DIRECTORYCOMPLETER_H