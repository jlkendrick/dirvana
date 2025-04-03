#ifndef RECENTLYACCESSEDCACHE_HPP
#define RECENTLYACCESSEDCACHE_HPP

#include "BaseCache.hpp"
#include "ISerializable.h"
#include "nlohmann/json.hpp"

#include <string>

using ordered_json = nlohmann::ordered_json;


// Recently accessed promotion policy
class RecentlyAccessedCache : public BaseCache<RACEntry> {
public:
	RACEntry create_entry(const std::string& path) const override {
		return RACEntry(path);
	}

	RACEntry create_entry(const ordered_json& entry) const override {
		return RACEntry(entry["path"].get<std::string>());
	}
			
	std::string get_path(const RACEntry& entry) const override {
		return entry.path;
	}

	void promote(const std::string& path) override {
		auto it = cache[path];
		RACEntry entry = *it;
			
		// Move the entry to the front of the list
		order.erase(it);
		order.push_front(entry);
		cache[path] = order.begin();
	}
};

#endif // RECENTLYACCESSEDCACHE_HPP