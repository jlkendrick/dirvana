#ifndef FREQUENCY_BASED_CACHE_HPP
#define FREQUENCY_BASED_CACHE_HPP

#include "ISerializable.h"
#include "nlohmann/json.hpp"

#include <string>

using ordered_json = nlohmann::ordered_json;


// Frequency-based promotion policy
class FrequencyBasedPolicy {
public:
	struct FBCEntry : public ISerializable {
		std::string path;
		int access_count;
		
		FBCEntry() : path(""), access_count(-1) {}
		FBCEntry(const std::string& p, int count = 0) : path(p), access_count(count) {}

		ordered_json serialize() const override {
			ordered_json j;
			j["path"] = path;
			j["access_count"] = access_count;
			return j;
		}
	};


	FBCEntry create_entry(const std::string& path) const {
		return FBCEntry(path);
	}

	FBCEntry create_entry(const ordered_json& entry) const {
		return FBCEntry(entry["path"].get<std::string>(), entry["access_count"].get<int>());
	}
		
	std::string get_path(const FBCEntry& entry) const {
		return entry.path;
	}
		
	template <typename CacheType>
	void promote(const std::string& path) {
		CacheType* self = static_cast<CacheType*>(this);
		auto it = self->cache[path];
		FBCEntry entry = *it;
			
		// Increment the access count
		entry.access_count++;
			
		// Remove the entry from its current position
		self->order.erase(it);
			
		// Find the position to insert based on access count (higher counts first)
		auto insert_pos = self->order.begin();
		while (insert_pos != self->order.end() && 
						insert_pos->access_count > entry.access_count) {
			++insert_pos;
		}
			
		// Insert the entry at the appropriate position
		auto new_it = self->order.insert(insert_pos, entry);
		self->cache[path] = new_it;
	}
};

using FBCEntry = FrequencyBasedPolicy::FBCEntry;

#endif // FREQUENCY_BASED_CACHE_HPP