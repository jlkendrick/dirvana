#ifndef FREQUENCY_BASED_CACHE_HPP
#define FREQUENCY_BASED_CACHE_HPP

#include "BaseCache.hpp"
#include "ISerializable.h"
#include "nlohmann/json.hpp"

#include <string>

using ordered_json = nlohmann::ordered_json;


// Frequency-based promotion policy
class FrequencyBasedCache : public BaseCache<FBCEntry> {
public:
	FBCEntry create_entry(const std::string& path) const override {
		return FBCEntry(path);
	}

	FBCEntry create_entry(const ordered_json& entry) const override {
		return FBCEntry(entry["path"].get<std::string>(), entry["access_count"].get<int>());
	}
		
	std::string get_path(const FBCEntry& entry) const override {
		return entry.path;
	}
		
	void promote(const std::string& path) override {
		auto it = cache[path];
		FBCEntry entry = *it;
			
		// Increment the access count
		entry.access_count++;
			
		// Remove the entry from its current position
		order.erase(it);
			
		// Find the position to insert based on access count (higher counts first)
		auto insert_pos = order.begin();
		while (insert_pos != order.end() && 
						insert_pos->access_count > entry.access_count) {
			++insert_pos;
		}
			
		// Insert the entry at the appropriate position
		auto new_it = order.insert(insert_pos, entry);
		cache[path] = new_it;
	}
};

#endif // FREQUENCY_BASED_CACHE_HPP