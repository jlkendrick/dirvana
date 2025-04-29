#ifndef HISTORYCACHE_H
#define HISTORYCACHE_H

#include "BaseCache.hpp"
#include "ISerializable.h"
#include <json.hpp>

// HistoryCache is a mix between the recently accessed cache and the frequency-based cache
// It stores the index of the path in the frequency-based style to allow for order comparisons in the priority queue
// It has the promotion method of the recently accessed cache so we can easily put the accessed path at the front
class HistoryCache : public BaseCache<HCEntry> {
public:
	HCEntry create_entry(const std::string& path) override { return HCEntry(path, current_index++); };
	HCEntry create_entry(const ordered_json& entry) override { return HCEntry(entry["path"].get<std::string>(), current_index++); }
	std::string get_path(const HCEntry& entry) const override { return entry.path; }
	void promote(const std::string& path) override;

	// HistoryCache specific methods
	int lookup_index(const std::string& path) const;
	int get_current_index() const { return current_index; } // For testing purposes

private:
	int current_index = 0; // Used to keep track of the current index for the next entry
};

#endif // HISTORYCACHE_H