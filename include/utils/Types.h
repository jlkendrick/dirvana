#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

// Exclusion types for directory names
enum class ExclusionType { Exact, Prefix, Suffix, Contains };
// Struct to hold the exclusion rules for directory names
struct ExclusionRule { ExclusionType type; std::string pattern; };

// Struct to hold the arguments for the DirectoryCompleter
struct DCArgs {
	bool build = true;
	bool refresh = false;
	std::string config_path = "";
	bool test_mode = false;
};

// Stores the available matching types for the cache
enum class MatchingType { Exact, Prefix, Suffix, Contains };

// Stores the available promotion strategies for the cache
enum class PromotionStrategy {
	RECENTLY_ACCESSED,
	FREQUENCY_BASED
};

#endif // TYPES_H