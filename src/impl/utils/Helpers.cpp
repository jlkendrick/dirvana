#include "Helpers.h"
#include "Types.h"

#include <string>
#include <iostream>

// Helper function to return the deepest directory name in a path
// Ex. get_deepest_dir("/Users/jameskendrick/Code/Projects/dirvana/cpp/src") will return "src"
// We return a pair of bool and string to ensure that the path is valid
std::pair<bool, std::string> get_deepest_dir(const std::string& path) {
	size_t pos = path.find_last_of('/');
	if (pos >= std::string::npos)
		return std::make_pair(false, "");

	return std::make_pair(true, path.substr(pos + 1));
}

MatchingType TypeConversions::s_to_matching_type(const std::string& type) {
	if (type == "exact") return MatchingType::Exact;
	else if (type == "prefix") return MatchingType::Prefix;
	else if (type == "suffix") return MatchingType::Suffix;
	else if (type == "contains") return MatchingType::Contains;
	else {
		std::cerr << "Unknown matching type: " << type << std::endl;
		return MatchingType::Exact;
	}
}

PromotionStrategy TypeConversions::s_to_promotion_strategy(const std::string& type) {
	if (type == "recently_accessed") return PromotionStrategy::RECENTLY_ACCESSED;
	else if (type == "frequency_based") return PromotionStrategy::FREQUENCY_BASED;
	else {
		std::cerr << "Unknown promotion strategy: " << type << std::endl;
		return PromotionStrategy::RECENTLY_ACCESSED;
	}
}

ExclusionRule TypeConversions::s_to_exclusion_rule(const std::string& excl_type, const std::string& excl_pattern) {
	ExclusionRule rule;
	if (excl_type == "prefix") {
		rule.type = ExclusionType::Prefix;
	} else if (excl_type == "exact") {
		rule.type = ExclusionType::Exact;
	} else if (excl_type == "suffix") {
		rule.type = ExclusionType::Suffix;
	} else if (excl_type == "contains") {
		rule.type = ExclusionType::Contains;
	} else {
		std::cerr << "Unknown exclusion pattern: " << excl_type << std::endl;
		return rule;
	}
	rule.pattern = excl_pattern;

	return rule;
}

ExclusionType TypeConversions::s_to_exclusion_type(const std::string& type) {
	if (type == "prefix") return ExclusionType::Prefix;
	else if (type == "exact") return ExclusionType::Exact;
	else if (type == "suffix") return ExclusionType::Suffix;
	else if (type == "contains") return ExclusionType::Contains;
	else {
		std::cerr << "Unknown exclusion type: " << type << std::endl;
		return ExclusionType::Exact;
	}
}

std::string TypeConversions::exclusion_type_to_s(const ExclusionType& type) {
	switch (type) {
		case ExclusionType::Prefix:
			return "prefix";
		case ExclusionType::Exact:
			return "exact";
		case ExclusionType::Suffix:
			return "suffix";
		case ExclusionType::Contains:
			return "contains";
		default:
			std::cerr << "Unknown exclusion type: " << static_cast<int>(type) << std::endl;
			return "exact";
	}
}

json TypeConversions::exclusion_rules_to_json(const std::vector<ExclusionRule>& rules) {
	json j;
	for (const auto& rule : rules) {
		std::string type = TypeConversions::exclusion_type_to_s(rule.type);
		if (j.contains(type)) {
			j[type].push_back(rule.pattern);
		} else {
			j[type] = json::array();
			j[type].push_back(rule.pattern);
		}
	}
	return j;
}

