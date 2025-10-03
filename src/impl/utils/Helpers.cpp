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

std::pair<bool, Flag> ArgParsing::build_flag(const std::vector<std::string>& flag_parts, const std::string& cmd) {
	Flag flag;

	flag.cmd = cmd;

	if (flag_parts.size() > 0) {
		std::string raw_flag = flag_parts[0];

		// Convert -fs to --flags
		if (raw_flag.starts_with("-") and not raw_flag.starts_with("--")) {
			// Make sure the short flag is a valid alias
			std::string short_flag = raw_flag.substr(1);
			if (ArgParsing::flag_aliases.find(short_flag) == ArgParsing::flag_aliases.end()) {
				std::cerr << "Invalid flag '-" << short_flag << "'" << std::endl;
				return {false, flag};
			}
			raw_flag = "--" + ArgParsing::flag_aliases.at(short_flag);
		}

		// Reject invalid flags
		if (raw_flag.starts_with("---")) {
			std::cerr << "Invalid flag '" << raw_flag << "'" << std::endl;
			return {false, flag};
		}

		// Convert flags to their implied names
		if (raw_flag_to_implied.find(raw_flag) != raw_flag_to_implied.end())
			raw_flag = raw_flag_to_implied.at(raw_flag);

		// Now we should have a flag starting with '--'
		std::string trimmed_flag = raw_flag.substr(2);
		if (std::find(full_flag_names.begin(), full_flag_names.end(), trimmed_flag) == full_flag_names.end()) {
			std::cerr << "Invalid flag '--" << trimmed_flag << "'" << std::endl;
			return {false, flag};
		}

		flag.flag = trimmed_flag;
	}

	if (flag_parts.size() > 1)
		flag.value = flag_parts[1];

	return {true, flag};
}

std::tuple<bool, std::string, std::vector<std::string>, std::vector<Flag>> ArgParsing::process_args(int argc, char* argv[]) {
	std::string call_type = argc > 1 ? argv[1] : ""; // --enter or --tab
	std::vector<std::string> cmd_parts;
	std::vector<Flag> flags;
	std::vector<std::string> curr_flag_parts;
	bool found_flag = false;

	// Start from index 3 to skip the program name, call type (--enter or --tab), and "dv"
	for (int i = 3; i < argc; i++) {
		std::string arg = argv[i];

		// Condition that indicates the start of a flag
		if (arg.starts_with("-") and arg.size() > 1) {

			// If we were already building a flag, save it
			if (found_flag and !curr_flag_parts.empty()) {
				std::string cmd = cmd_parts.empty() ? "" : cmd_parts.back(); // We associate the flag with the last command part
				auto [success, flag] = ArgParsing::build_flag(curr_flag_parts, cmd);

				if (success && ArgParsing::validate_flag(flag))
					flags.push_back(flag);
				else
					// If the flag is invalid, we stop processing further
					return {false, "", {}, {}};
				curr_flag_parts.clear();
			} else {
				found_flag = true;
			}
		}

		// After we start building a flag, all subsequent args belong to the flag (or another flag)
		if (found_flag)
			curr_flag_parts.push_back(arg);
		else
			cmd_parts.push_back(arg);
	}

	// If we ended while building a flag, save it
	if (found_flag and !curr_flag_parts.empty()) {
		auto [success, flag] = ArgParsing::build_flag(curr_flag_parts, cmd_parts.empty() ? "" : cmd_parts.back());
		if (success && ArgParsing::validate_flag(flag))
			flags.push_back(flag);
		else
			// If the flag is invalid, we stop processing
			return {false, "", {}, {}};
	}

	return {true, call_type, cmd_parts, flags};
}

bool ArgParsing::validate_flag(const Flag& flag) {
	// Determine which set of valid flags to use based on the associated command
	std::string associated_cmd = flag.cmd;
	if (ArgParsing::valid_flags.find(associated_cmd) == ArgParsing::valid_flags.end())
		return false;
	const auto& flags = ArgParsing::valid_flags.at(associated_cmd);

	// Check if the flag is valid for the associated command
	auto it = std::find_if(flags.begin(), flags.end(), [&flag](const auto& pair) {
		return pair.first == flag.flag;
	});
	if (it == flags.end()) {
		std::cerr << "Invalid flag '--" << flag.flag << "' for command '" << associated_cmd << "'" << std::endl;
		return false;
	}

	bool requires_value = it->second;
	if (requires_value && flag.value.empty()) {
		std::cerr << "Flag --" << flag.flag << " requires a value" << std::endl;
		return false;
	}

	if (!requires_value && !flag.value.empty()) {
		std::cerr << "Flag --" << flag.flag << " does not take a value" << std::endl;
		return false;
	}

	return true;
}

std::string ArgParsing::get_flag_value(const std::vector<Flag>& flags, const std::string& flag_name, const std::string& default_value) {
	for (const auto& flag : flags) {
		if (flag.flag == flag_name) {
			return flag.value;
		}
	}
	return default_value;
}

bool ArgParsing::has_flag(const std::vector<Flag>& flags, const std::string& flag_name) {
	for (const auto& flag : flags) {
		if (flag.flag == flag_name) {
			return true;
		}
	}
	return false;
}