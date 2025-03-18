#ifndef BASECACHE_H
#define BASECACHE_H

#include <iostream>
#include <string>
#include <list>
#include <unordered_map>

template <typename T>
class BaseCache {
public:
	
	void add(const std::string& key, const T& value = T());
	void remove(const std::string& key);
	void access(const std::string& entry);
	virtual void promote(const std::string& entry);

	typename std::list<T>::const_iterator get_iter() const { return order.cbegin(); }
	const std::list<T>& get_list() const { return order; }
	std::vector<T> get_all_entries() const;

	bool contains(const std::string& entry) const { return cache.find(entry) != cache.end(); }
	int get_size() const { return size; }
	
protected:
	std::unordered_map<std::string, typename std::list<T>::iterator> cache;
	std::list<T> order;

	int size = 0;
};

template <typename T>
void BaseCache<T>::add(const std::string& key, const T& value) {
	// If the key is already in the cache, do nothing
	if (contains(key))
		return;

	if (value.empty())
		order.push_back(key);
	else
		order.push_back(value);
	cache.insert({key, --order.end()}); // Entry is guaranteed to be unique here

	size++;
}

template <typename T>
void BaseCache<T>::remove(const std::string& key) {
	// If the entry is not in the cache, do nothing
	if (!contains(key))
		return;

	auto it = cache.at(key);

	cache.erase(key);
	order.erase(it);

	size--;
}

template <typename T>
void BaseCache<T>::access(const std::string& key) {
	// If the key is already in the cache, promote it
	if (contains(key)) {
		promote(key);
		return;
	}

	add(key);
}

template <typename T>
void BaseCache<T>::promote(const std::string& entry) {
	// This is a no-op for the base cache
	std::cout << "This should not be called" << std::endl;
}

template <typename T>
std::vector<T> BaseCache<T>::get_all_entries() const {
	return std::vector<T>(order.begin(), order.end());
}

#endif // BASECACHE_H