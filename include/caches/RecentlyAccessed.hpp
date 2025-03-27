#ifndef RECENTLYACCESSEDCACHE_HPP
#define RECENTLYACCESSEDCACHE_HPP

#include "Serializable.h"
#include "nlohmann/json.hpp"

#include <string>

using json = nlohmann::json;


// Recently accessed promotion policy
class RecentlyAccessedPolicy {
public:
	struct RACEntry : public ISerializable {
		std::string path;

		RACEntry() : path("") {}
		RACEntry(const std::string& p) : path(p) {}

		json serialize() const override {
			json j;
			j["path"] = path;
			return j;
		}
	};
	

	RACEntry create_entry(const std::string& path) const {
		return RACEntry(path);
	}
			
	std::string get_path(const RACEntry& entry) const {
		return entry.path;
	}
			
	template <typename CacheType>
	void promote(const std::string& path) {
		CacheType* self = static_cast<CacheType*>(this);
		auto it = self->cache[path];
		RACEntry entry = *it;
			
		// Move the entry to the front of the list
		self->order.erase(it);
		self->order.push_front(entry);
		self->cache[path] = self->order.begin();
	}
};

using RACEntry = RecentlyAccessedPolicy::RACEntry;

#endif // RECENTLYACCESSEDCACHE_HPP