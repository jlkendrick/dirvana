#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <vector>

std::pair<bool, std::string> get_deepest_dir(const std::string& path);
enum class ExclusionType { Exact, Prefix, Suffix, Contains };
struct ExclusionRule { ExclusionType type; std::string pattern; };
struct DCArgs {
	bool build = true;
	bool refresh = false;
	std::string config_path = "";
	std::vector<ExclusionRule> exclude = {};
};
enum class MatchingType { Exact, Prefix, Suffix };

#endif // HELPERS_H