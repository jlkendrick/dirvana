#ifndef HELPERS_H
#define HELPERS_H

#include "caches/FrequencyBasedCache.hpp"
#include "caches/RecentlyAccessedCache.hpp"
#include "caches/interfaces/ICache.h"
#include "nlohmann/json.hpp"
#include "Types.h"

#include <string>
#include <vector>

using json = nlohmann::json;

std::pair<bool, std::string> get_deepest_dir(const std::string& path);
std::string extract_promotion_strategy(const std::string& dirname);

namespace TypeConversions {
	MatchingType s_to_matching_type(const std::string& type);

	PromotionStrategy s_to_promotion_strategy(const std::string& type);

	ExclusionRule s_to_exclusion_rule(const std::string& excl_type, const std::string& excl_pattern);
	ExclusionType s_to_exclusion_type(const std::string& type);
	json exclusion_rules_to_json(const std::vector<ExclusionRule>& rules);
	std::string exclusion_type_to_s(const ExclusionType& type);
};

namespace ListUtils {
	// Merges k sorted lists into a single sorted list, based on the 
}

// Factory for creating the appropriate cache type
namespace CacheFactory {
	std::unique_ptr<ICache> create_cache(PromotionStrategy strategy);
};

#endif // HELPERS_H