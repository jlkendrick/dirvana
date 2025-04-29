#ifndef FREQUENCY_BASED_CACHE_HPP
#define FREQUENCY_BASED_CACHE_HPP

#include "BaseCache.hpp"
#include "ISerializable.h"
#include <json.hpp>

using ordered_json = nlohmann::ordered_json;


// Frequency-based promotion policy
class FrequencyBasedCache : public BaseCache<FBCEntry> {
public:
	FBCEntry create_entry(const std::string& path) override { return FBCEntry(path); }
	FBCEntry create_entry(const ordered_json& entry) override { return FBCEntry(entry["path"].get<std::string>(), entry["access_count"].get<int>()); }
	std::string get_path(const FBCEntry& entry) const override { return entry.path; }
	void promote(const std::string& path) override;
};

#endif // FREQUENCY_BASED_CACHE_HPP