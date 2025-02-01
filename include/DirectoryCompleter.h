#ifndef DIRECTORYCOMPLETER_H
#define DIRECTORYCOMPLETER_H

#include "RecentlyAccessedCache.h"
#include "PathMap.h"
#include "DLLTraverser.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

class DirectoryCompleter;
struct DCArgs;

class DirectoryCompleter {
public:

	// Exclusion rules for directories
	enum class ExclusionType {
		Exact,
		Prefix,
		Suffix,
		Contains
	};
	struct ExclusionRule {
		ExclusionType type;
		std::string pattern;
	};

	// Constructs a new DirectoryCompleter with the specified variables or the default values
	DirectoryCompleter(const DCArgs& args);

	~DirectoryCompleter() = default;

	// Returns the doubly linked list that can be used to construct the traverser
	const std::shared_ptr<DoublyLinkedList> get_list_for(const std::string& dir) const;

	bool has_matches(const std::string& dir) const { return directories.contains(dir); }

	// Finds the matching cache for the given directory name and returns the paths in that cache
	std::vector<std::string> get_all_matches(const std::string& dir) const;

	// Indicates that the given path has been accessed and it's position in the cache should be updated
	void access(const std::string& path) { directories.add(path); }

	// Returns the number of directories in the completer
	int get_size() const { return directories.get_size(); }

	// Adds an exclusion rule to the completer
	void add_exclusion_rule(const ExclusionRule& rule) { exclusion_rules.push_back(rule); }

	// Functions to save and load the completer from a JSON file
	void save() const;
	void load();

private:
	std::string rootdir;
	PathMap directories;

	std::string init_path = std::getenv("HOME");
	std::string cache_path = "cache.json";


	// Returns true if the given directory should be excluded from the PathMap
	bool should_exclude(const std::string& dir) const;

	// Helper function to get the deepest directory name of a given path
	std::string get_deepest_dir(const std::string& path) const;

	// Private function used by the constructor to collect all of the directories in the root directory
	void collect_directories();


	// Stores excludsion rules for directories
	std::vector<ExclusionRule> exclusion_rules = {
		// Exclude dot directories
		{ ExclusionType::Prefix, "." },

		// Exclude common auto-generated directories
		{ ExclusionType::Exact, "node_modules" },
		{ ExclusionType::Exact, "bower_components" },
		{ ExclusionType::Exact, "build" },
		{ ExclusionType::Exact, "dist" },
		{ ExclusionType::Exact, "out" },
		{ ExclusionType::Exact, "target" },
		{ ExclusionType::Exact, "tmp" },
		{ ExclusionType::Exact, "temp" },
		{ ExclusionType::Exact, "cache" },
		{ ExclusionType::Exact, "venv" },
		{ ExclusionType::Exact, "env" },
		{ ExclusionType::Exact, ".env" },
		{ ExclusionType::Exact, "obj" },
		
		// Suffix rule: Exclude directories ending with "sdk"
		{ ExclusionType::Suffix, "sdk" },

		// Contains rule: Exclude directories containing "release"
		{ ExclusionType::Contains, "release" }
	};
};

struct DCArgs {
	bool build = true;
	std::string init_path = "";
	std::vector<DirectoryCompleter::ExclusionRule> exclude = {};
	std::string cache_path = "";
};

#endif // DIRECTORYCOMPLETER_H