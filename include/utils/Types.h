#ifndef TYPES_H
#define TYPES_H

#include "nlohmann/json.hpp"
#include "ISerializable.h"

#include <string>
#include <vector>

using ordered_json = nlohmann::ordered_json;


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

// Struct to hold the recently accessed cache entry
struct RACEntry : public ISerializable {
	std::string path;

	RACEntry() : path("") {}
	RACEntry(const std::string& p) : path(p) {}

	ordered_json serialize() const override;
};

// Struct to hold the frequency-based cache entry
struct FBCEntry : public ISerializable {
	std::string path;
	int access_count;
	
	FBCEntry() : path(""), access_count(-1) {}
	FBCEntry(const std::string& p, int count = 0) : path(p), access_count(count) {}

	ordered_json serialize() const override;
};

#endif // TYPES_H