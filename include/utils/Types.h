#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

enum class ExclusionType { Exact, Prefix, Suffix, Contains };
struct ExclusionRule { ExclusionType type; std::string pattern; };
struct DCArgs {
	bool build = true;
	bool refresh = false;
	std::string config_path = "";
	std::vector<ExclusionRule> exclude = {};
};
enum class MatchingType { Exact, Prefix, Suffix };

#endif // TYPES_H