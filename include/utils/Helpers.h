#ifndef HELPERS_H
#define HELPERS_H

#include <json.hpp>
#include "Types.h"

#include <string>
#include <vector>

using json = nlohmann::json;

std::string get_dir_name(const std::string& path);
std::string extract_promotion_strategy(const std::string& dirname);

namespace TypeConversions {
	MatchingType s_to_matching_type(const std::string& type);

	PromotionStrategy s_to_promotion_strategy(const std::string& type);

	ExclusionRule s_to_exclusion_rule(const std::string& excl_type, const std::string& excl_pattern);
	ExclusionType s_to_exclusion_type(const std::string& type);
	json exclusion_rules_to_json(const std::vector<ExclusionRule>& rules);
	std::string exclusion_type_to_s(const ExclusionType& type);
};

namespace ArgParsing {
	static std::unordered_map<std::string, std::string> flag_aliases = {
		{"v", "version"},
		{"h", "help"},
		{"r", "root"},
		{"e", "exact"},
		{"p", "prefix"},
		{"s", "suffix"},
		{"c", "contains"},
		{"ra", "recently_accessed"},
		{"fb", "frequency_based"}
	};
	static std::vector<std::string> full_flag_names = {
		"version",
		"help",
		"root",
		"exact",
		"prefix",
		"suffix",
		"contains",
		"recently_accessed",
		"frequency_based"
	};
	const std::unordered_map<std::string, std::vector<std::pair<std::string, bool>>> valid_flags = {
		// To be implemented later
		// {
		// 		"[none]", {
		// 			{"help", false}, {"h", false}
		// 		}
		// 	},
		// {
		// 		"[query]", {
		// 			// One time matching type override flags
		// 			{"exact", false}, {"e", false},
		// 			{"prefix", false}, {"p", false},
		// 			{"suffix", false}, {"s", false},
		// 			{"contains", false}, {"c", false},
		// 			// One time promotion strategy override flags
		// 			{"recently_accessed", false}, {"ra", false},
		// 			{"frequency_based", false}, {"fb", false}
		// 		}
		// 	},
		// Build/rebuild/refresh command flags
	{"", {{"version", false}}},
	{"build", {{"root", true}}},
	{"rebuild", {{"root", true}}},
	{"refresh", {{"root", true}}}
	};
	std::pair<bool, Flag> build_flag(const std::vector<std::string>& flag_parts, const std::string& cmd);
	std::tuple<bool, std::string, std::vector<std::string>, std::vector<Flag>> process_args(int argc, char* argv[]);
	bool validate_flag(const Flag& flag);
	std::string get_flag_value(const std::vector<Flag>& flags, const std::string& flag_name, const std::string& default_value = "");
	bool has_flag(const std::vector<Flag>& flags, const std::string& flag_name);
}

#endif // HELPERS_H