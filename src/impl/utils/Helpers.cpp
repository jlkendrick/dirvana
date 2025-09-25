#include "Helpers.h"
#include "Types.h"

#include <string>
#include <iostream>

// Helper function to return the deepest directory name in a path
// Ex. get_deepest_dir("/Users/jameskendrick/Code/Projects/dirvana/cpp/src") will return "src"
// We return a pair of bool and string to ensure that the path is valid
std::string get_dir_name(const std::string& path) {
	size_t pos = path.find_last_of('/');

	// If there is no '/' in the path, return the path itself
	if (pos == std::string::npos)
		return path;
	
	return path.substr(pos + 1);
}

std::string extract_promotion_strategy(const std::string& dirname) {
	size_t pos = dirname.find_first_of('-');
	if (pos == std::string::npos)
		return "";
	
	return dirname.substr(0, pos);
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
	if (rules.empty())
		return json::object();

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

Flag ArgParsing::build_flag(const std::vector<std::string>& flag_parts, const std::string& cmd) {
	Flag flag;
	flag.cmd = cmd;
	if (flag_parts.size() > 0)
		flag.flag = flag_parts[0].substr(2); // Remove the "--" prefix
	if (flag_parts.size() > 1)
		flag.value = flag_parts[1];
	return flag;
}

std::pair<std::vector<std::string>, std::vector<Flag>> ArgParsing::split_cmd_and_flags(int argc, char* argv[]) {
	std::vector<std::string> cmd_parts;
	std::vector<Flag> flags;
	std::vector<std::string> curr_flag_parts;
	bool found_flag = false;

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];

		// Condition that indicates the start of a flag
		if (arg.starts_with("--") and arg.size() > 2) {
			// If we were already building a flag, save it
			if (found_flag and !curr_flag_parts.empty()) {
				std::string cmd = cmd_parts.empty() ? "" : cmd_parts.back(); // We associate the flag with the last command part
				Flag flag = ArgParsing::build_flag(curr_flag_parts, cmd);
				flags.push_back(flag);
				curr_flag_parts.clear();
			} else
				found_flag = true;
		}

		// After we start building a flag, all subsequent args belong to the flag (or another flag)
		if (found_flag)
			curr_flag_parts.push_back(arg);
		else
			cmd_parts.push_back(arg);
	}

	// If we ended while building a flag, save it
	if (found_flag and !curr_flag_parts.empty()) {
		Flag flag = ArgParsing::build_flag(curr_flag_parts, cmd_parts.empty() ? "" : cmd_parts.back());
		flags.push_back(flag);
	}

	return {cmd_parts, flags};
}