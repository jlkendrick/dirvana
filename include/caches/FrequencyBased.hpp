#ifndef FREQUENCY_BASED_CACHE_HPP
#define FREQUENCY_BASED_CACHE_HPP

#include "Serializable.h"
#include "nlohmann/json.hpp"

#include <string>

using json = nlohmann::json;


// Frequency-based promotion policy
class FrequencyBasedPolicy {
public:
	struct FBCEntry : public ISerializable {
		std::string path;
		int access_count;
		
		FBCEntry() : path(""), access_count(-1) {}
		FBCEntry(const std::string& p) : path(p), access_count(1) {}

		json serialize() const override {
			json j;
			j["path"] = path;
			j["access_count"] = access_count;
			return j;
		}
	};
	

	FBCEntry create_entry(const std::string& path) const {
		return FBCEntry(path);
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