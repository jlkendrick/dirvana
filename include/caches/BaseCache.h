#ifndef BASECACHE_H
#define BASECACHE_H

#include <list>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>

// Stores the available promotion strategies for the cache
enum class PromotionStrategy {
	RECENTLY_ACCESSED,
	FREQUENCY_BASED
};

// Struct to hold the path for the RecentlyAccessedCache
struct RACEntry {
	std::string path;

	RACEntry() : path("") {};
	RACEntry(const std::string& n) : path(n) {}

	bool empty() const { return path.empty(); }
};

// Struct to hold both the path and its access count for frequency-based caching
struct FBCEntry {
	std::string path;
	int access_count;

	FBCEntry() : path(""), access_count(-1) {};
	FBCEntry(const std::string& n) : path(n), access_count(1) {}

	bool empty() const { return path.empty() && access_count == -1; }
};

// Generic base class for caches. Takes the type of the cache entry and the promotion strategy as template parameters
// This allows us to use the same interface for both RecentlyAccessedCache and FrequencyBasedCache
template <typename K, typename V>
class BaseCache {
public:
	BaseCache() = default;
	BaseCache(const PromotionOption& strat) : size(0), strategy(strat) {}

	void add(const std::string& path);
	void remove(const std::string& key);
	void access(const std::string& entry);
	void promote(const std::string& entry);

	typename std::list<T>::const_iterator get_iter() const { return order.cbegin(); }
	const std::list<T>& get_list() const { return order; }
	std::vector<std::string> get_all_paths() const;

	// Util functions used for testing
	bool contains(const std::string& entry) const { return cache.find(entry) != cache.end(); }
	int get_size() const { return size; }

protected:
	std::unordered_map<std::string, typename std::list<T>::iterator> cache;
	std::list<T> order;

	// PromotionOption strategy = PromotionOption::RECENTLY_ACCESSED;

	int size = 0;
};

template <typename K, typename V>
void BaseCache<K, V>::add(const std::string& path) {
	// If the path is already in the cache, do nothing
	if (contains(path))
		return;

	// Create the new entry and add it to the cache and order
	T new_entry(path);
	order.push_back(new_entry);
	cache[path] = --order.end();

	size++;
}

template <typename K, typename V>
void BaseCache<K, V>::remove(const std::string& key) {
	// If the entry is not in the cache, do nothing
	if (!contains(key))
		return;

	auto it = cache.at(key);

	cache.erase(key);
	order.erase(it);

	size--;
}

template <typename K, typename V>
void BaseCache<K, V>::access(const std::string& key) {
	// If the key is already in the cache, promote it
	if (contains(key)) {
		promote(key);
		return;
	}

	add(key);
}

template <typename K, typename V>
void BaseCache<K, V>::promote(const std::string& entry) {
	auto it = cache.at(entry);

	if constexpr (std::is_same_v<V, RACEntry>) {
		auto it = cache.at(path);
		std::string node = *it;
		order.erase(it);
		order.push_front(node);
		cache[path] = order.begin();

	} else if constexpr (std::is_same_v<V, FBCEntry>) {
		auto it = cache.at(path);
		FBCEntry updated_entry = *it;
		updated_entry.access_count++;
		order.erase(it);
		auto insert_pos = order.begin();
		while (insert_pos != order.end() && insert_pos->access_count > updated_entry.access_count)
				insert_pos++;
		auto new_it = order.insert(insert_pos, updated_entry);
		cache[path] = new_it;
	}
}

template <typename K, typename V>
std::vector<std::string> BaseCache<K, V>::get_all_paths() const {
	std::vector<K> paths;

	// Map the order to a vector of strings
	if constexpr (std::is_same<T, FBCEntry>::value)
		std::transform(order.begin(), order.end(), std::back_inserter(paths), [](const FBCEntry& entry) { return entry.path; });
	else
		paths.assign(order.begin(), order.end());

	return paths;
}

#endif // BASECACHE_H