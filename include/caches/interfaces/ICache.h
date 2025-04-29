#ifndef ICACHE_HPP
#define ICACHE_HPP

#include <json.hpp>

#include <string>
#include <vector>

using ordered_json = nlohmann::ordered_json;


class ICache {
public:
	virtual ~ICache() = default;
	virtual void add(const std::string& path) = 0;
	virtual void add(const ordered_json& entry) = 0;
	virtual void remove(const std::string& path) = 0;
	virtual void access(const std::string& path) = 0;
	virtual std::vector<std::string> get_paths(unsigned int max_results) const = 0;
	virtual std::vector<std::string> get_all_paths() const = 0;
	virtual ordered_json serialize_entries(int max_results = -1) const = 0;
	virtual bool contains(const std::string& path) const = 0;
	virtual int get_size() const = 0;
};

#endif // ICACHE_HPP