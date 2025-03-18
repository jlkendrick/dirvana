#ifndef BASECACHE_H
#define BASECACHE_H

#include <list>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>

// Stores the available promotion strategies for the cache
enum class PromotionOption {
	RECENTLY_ACCESSED,
	FREQUENCY_BASED
};

// Struct to hold both the path and its access count for frequency-based caching
struct FBCEntry {
	std::string path;
	int access_count;

	FBCEntry() : path(""), access_count(-1) {};
	FBCEntry(const std::string& n) : path(n), access_count(1) {}

	bool empty() const { return path.empty() && access_count == -1; }
};

struct RecentlyAccessedPromotion;
struct FrequencyBasedPromotion;

// Generic base class for caches. Takes the type of the cache entry and the promotion strategy as template parameters
// This allows us to use the same interface for both RecentlyAccessedCache and FrequencyBasedCache
template <typename T, typename PromotionStrategy>
class BaseCache {
public:
	BaseCache() = default;

	void add(const std::string& key, const T& value = T());
	void remove(const std::string& key);
	void access(const std::string& entry);

	void promote(const std::string& entry) {
		static_cast<PromotionStrategy*>(this)->promote_impl(*this, entry);
	};

	typename std::list<T>::const_iterator get_iter() const { return order.cbegin(); }
	const std::list<T>& get_list() const { return order; }
	std::vector<std::string> get_all_paths() const;

	// Util functions used for testing
	bool contains(const std::string& entry) const { return cache.find(entry) != cache.end(); }
	int get_size() const { return size; }
	
	struct RecentlyAccessedPromotion {
		void promote_impl(BaseCache<T, PromotionStrategy>& baseCache, const std::string& path) {
			auto it = baseCache.cache.at(path);
			std::string node = *it;
			baseCache.order.erase(it);
			baseCache.order.push_front(node);
			baseCache.cache[path] = baseCache.order.begin();
		}
	};
	
	struct FrequencyBasedPromotion {
		void promote_impl(BaseCache<T, PromotionStrategy>& baseCache, const std::string& path) {
			auto it = baseCache.cache.at(path);
			FBCEntry updated_entry = *it;
			updated_entry.access_count++;
			
			baseCache.order.erase(it);
			auto insert_pos = baseCache.order.begin();
			while (insert_pos != baseCache.order.end() && insert_pos->access_count > updated_entry.access_count)
					insert_pos++;
			auto new_it = baseCache.order.insert(insert_pos, updated_entry);
			baseCache.cache[path] = new_it;
		}
	};

protected:
	std::unordered_map<std::string, typename std::list<T>::iterator> cache;
	std::list<T> order;

	int size = 0;
};

template <typename T, typename PromotionStrategy>
void BaseCache<T, PromotionStrategy>::add(const std::string& key, const T& value) {
	// If the key is already in the cache, do nothing
	if (contains(key))
		return;

	// Note that we defined .empty() for FBCEntry, so we can use it here
	// If only the key is passed, we infer that we are using the RecentlyAccessedCache so we use the key as the value
	// If the value is passed, we are using the FrequencyBasedCache so we use the FBCEntry as the value
	order.push_back(value.empty() ? key : value);
	cache.insert({key, --order.end()});

	size++;
}

template <typename T, typename PromotionStrategy>
void BaseCache<T, PromotionStrategy>::remove(const std::string& key) {
	// If the entry is not in the cache, do nothing
	if (!contains(key))
		return;

	auto it = cache.at(key);

	cache.erase(key);
	order.erase(it);

	size--;
}

template <typename T, typename PromotionStrategy>
void BaseCache<T, PromotionStrategy>::access(const std::string& key) {
	// If the key is already in the cache, promote it
	if (contains(key)) {
		promote(key);
		return;
	}

	add(key);
}

template <typename T, typename PromotionStrategy>
std::vector<std::string> BaseCache<T, PromotionStrategy>::get_all_paths() const {
	std::vector<T> paths;

	// Map the order to a vector of strings
	if constexpr (std::is_same<T, FBCEntry>::value)
		std::transform(order.begin(), order.end(), std::back_inserter(paths), [](const FBCEntry& entry) { return entry.path; });
	else
		paths.assign(order.begin(), order.end());

	return paths;
}

#endif // BASECACHE_H