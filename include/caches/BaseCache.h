#ifndef BASECACHE_H
#define BASECACHE_H

#include <string>
#include <list>
#include <unordered_map>

template <typename T>
class BaseCache {
public:
	
	void add(const T& entry);
	void remove(const T& entry);
	void access(const T& entry);
	virtual void promote(const T& entry);

	typename std::list<T>::const_iterator get_iter() const { return order.cbegin(); }
	const std::list<T>& get_list() const { return order; }
	std::vector<T> get_all_entries() const;

	bool contains(const T& entry) const { return cache.find(entry) != cache.end(); }
	int get_size() const { return size; }
	
	std::unordered_map<T, typename std::list<T>::iterator> cache;
	std::list<T> order;

private:
	int size = 0;
};

template <typename T>
void BaseCache<T>::add(const T& entry) {
	// If the entry is already in the cache, do nothing
	if (contains(entry))
		return;

	order.push_back(entry);
	cache.insert({entry, --order.end()}); // Entry is guaranteed to be unique here

	size++;
}

template <typename T>
void BaseCache<T>::remove(const T& entry) {
	// If the entry is not in the cache, do nothing
	if (!contains(entry))
		return;

	auto it = cache.at(entry);

	cache.erase(entry);
	order.erase(it);

	size--;
}

template <typename T>
void BaseCache<T>::access(const T& entry) {
	// If the entry is already in the cache, promote it
	if (contains(entry)) {
		promote(entry);
		return;
	}

	add(entry);
}

template <typename T>
void BaseCache<T>::promote(const T& entry) {
	// This is a no-op for the base cache
}

template <typename T>
std::vector<T> BaseCache<T>::get_all_entries() const {
	return std::vector<T>(order.begin(), order.end());
}

#endif // BASECACHE_H