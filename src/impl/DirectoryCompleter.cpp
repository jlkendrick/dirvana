#include "DirectoryCompleter.h"

#include "Helpers.h"
#include "nlohmann/json.hpp"

#include <memory>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_set>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
namespace fs = std::filesystem;

DirectoryCompleter::DirectoryCompleter(const DCArgs& args) {
	// If we were given a config path, use it (should only be used for testing, not available in the main program)
	if (!args.config_path.empty())
		this->config_path = args.config_path;
	// Load the config file
	std::cout << "Using config path: " << this->config_path << std::endl;
	this->config = load_config();

	// Update the exclusion rules if any were passed
	if (!args.exclude.empty())
		this->exclusion_rules = args.exclude;
	
	// If we are building the cache, we add every directory in the root directory to the PathMap
	if (args.build) {
		for (const auto& [path, dir] : collect_directories())
			directories.add(path, dir);
		
	// If we are not building the cache, we need to load the cache from the file and, if refreshing, update the cache
	} else {
		std::unordered_set<std::string> old_dirs;
		load(old_dirs);
		if (args.refresh)
			// If we are refreshing, we need to compare the old vs new directories and update the cache accordingly
			refresh_directories(old_dirs);
	}
}

std::vector<std::pair<std::string, std::string>> DirectoryCompleter::collect_directories() {
	std::vector<std::pair<std::string, std::string>> dirs;
	try {
		fs::recursive_directory_iterator it(config["paths"]["init"].get<std::string>(), fs::directory_options::skip_permission_denied);
		fs::recursive_directory_iterator end;
		
		while (it != end) {
			const auto& entry = *it;

			// Get the deepest directory name
			std::pair<bool, std::string> res = get_deepest_dir(entry.path().string());

			// If the entry is a directory and it's not in the exclude list, add it to the PathMap
			if (fs::is_directory(entry)) {
				std::string dir_name = res.second;
				if (res.first && !should_exclude(dir_name, entry.path().string()))
					dirs.push_back({entry.path().string(), dir_name});
				
				// Otherwise, don't add it to the PathMap and disable recursion into it's children
				else
					it.disable_recursion_pending();
			}

			++it;
		}
		
	} catch (const fs::filesystem_error& e) {
		std::cerr << e.what() << std::endl;
	}

	return dirs;
}

void DirectoryCompleter::refresh_directories(std::unordered_set<std::string>& old_dirs) {
	// Go through the updated list of directories to see which ones are new
	for (const auto& [path, dir] : collect_directories()) {
		// If the directory is new, add it to the PathMap
		if (old_dirs.find(path) == old_dirs.end())
			directories.add(path, dir);
		
		// Otherwise, remove it from the set of old directories
		else
			old_dirs.erase(path);
	}

	// Now, old_dirs contains the directories that were removed so remove them from the PathMap
	for (const std::string& dir : old_dirs)
		directories.remove(dir);
}

void DirectoryCompleter::save() const {
	json j;
	for (const auto& [dir, cache] : directories.map) {
		json entry;
		entry["dir"] = dir;
		

		ordered_json paths = ordered_json::array();
		for (const auto& path : cache.get_all_paths())
			paths.push_back(path);
		entry["paths"] = paths;

		j.push_back(entry);
	}
	std::ofstream file(config["paths"]["cache"].get<std::string>());
	if (file.is_open()) {
		file << j.dump(4);
		file.close();
	} else {
		std::cerr << "Unable to open file for writing: " << config["paths"]["cache"].get<std::string>() << std::endl;
	}
}

void DirectoryCompleter::load(std::unordered_set<std::string>& old_dirs) {
	std::ifstream file(config["paths"]["cache"].get<std::string>());
	if (file.is_open()) {
		json j;
		file >> j;
		file.close();

		for (const auto& entry : j) {
			std::string dir = entry["dir"];
			ordered_json paths = entry["paths"];

			for (const auto& path : paths) {
				directories.add(path.get<std::string>(), dir);
				old_dirs.insert(path.get<std::string>());
			}
		}
	} else
		std::cerr << "Unable to open file for reading: " << config["paths"]["cache"].get<std::string>() << std::endl;
}

bool DirectoryCompleter::should_exclude(const std::string& dirname, const std::string& path) const {
	// Check if the directory should be excluded based on the exclusion rules
	for (const auto& rule : exclusion_rules) {
		switch (rule.type) {
      case ExclusionType::Exact:
        if (dirname == rule.pattern || path == rule.pattern)
					return true;
				break;

      case ExclusionType::Prefix:
				if (dirname.size() >= rule.pattern.size() &&
						dirname.compare(0, rule.pattern.size(), rule.pattern) == 0)
					return true;
				break;
        
      case ExclusionType::Suffix:
				if (dirname.size() >= rule.pattern.size() &&
						dirname.compare(dirname.size() - rule.pattern.size(), rule.pattern.size(), rule.pattern) == 0)
					return true;
				break;
        
			case ExclusionType::Contains:
				if (dirname.find(rule.pattern) != std::string::npos)
					return true;
				break;
				
		}
	}

	return false;
}

MatchingType DirectoryCompleter::s_to_matching_type(const std::string& type) const {
	if (type == "exact") return MatchingType::Exact;
	else if (type == "prefix") return MatchingType::Prefix;
	else if (type == "suffix") return MatchingType::Suffix;
	else {
		std::cerr << "Unknown matching type: " << type << std::endl;
		return MatchingType::Exact;
	}
}

json DirectoryCompleter::load_config() const {
	// Check if the config file exists
	if (!fs::exists(config_path)) {
		std::cerr << "Config file does not exist: " << config_path << ". Creating a new one." << std::endl;
		std::ofstream out_file(config_path);
		out_file << default_config.dump(4);
		out_file.close();
		return default_config;
	}

	try {
		std::ifstream in_file(config_path);
		if (!in_file.is_open()) {
			std::cerr << "Unable to open config file: " << config << std::endl;
			return default_config;
		}

		json user_config;
		in_file >> user_config;
		in_file.close();

		std::cout << user_config.dump(4) << std::endl;

		// Validate the config file
		if (validate_config(user_config)) {
			std::cerr << "Config file invalid. Fixing it with default values." << std::endl;
			std::ofstream out_file(config_path);
			out_file << user_config.dump(4);
			out_file.close();
		}

		std::cout << user_config.dump(4) << std::endl;

		// Return the (maybe) modified user config that is valid
		return user_config;

	} catch (const std::exception& e) {
		std::cerr << "Error reading config file: " << e.what() << ". Using default config." << std::endl;
		std::ofstream out_file(config_path);
		out_file << default_config.dump(4);
		out_file.close();
		return default_config;
	}
}

bool DirectoryCompleter::validate_config(json& user_config) const {
	bool modified = false;

	// If "paths" key is missing, add it
	if (!user_config.contains("paths")) {
		user_config["paths"] = default_config["paths"];
		modified = true;
	} else {
		// Now check the sub-keys to see if they are present and valid
		if (!user_config["paths"].contains("init") || (!fs::exists(user_config["paths"]["init"].get<std::string>()))) {
			user_config["paths"]["init"] = default_config["paths"]["init"];
			modified = true;
		}

		if (!user_config["paths"].contains("cache") || (!fs::exists(user_config["paths"]["cache"].get<std::string>()))) {
			user_config["paths"]["cache"] = default_config["paths"]["cache"];
			modified = true;
		}
	}


	// If "matching" key is missing, add it
	if (!user_config.contains("matching")) {
		user_config["matching"] = default_config["matching"];
		modified = true;
	} else {
		// Now check the sub-keys to see if they are present and valid
		if (!user_config["matching"].contains("max_results") || (user_config["matching"]["max_results"].get<int>() <= 0)) {
			user_config["matching"]["max_results"] = default_config["matching"]["max_results"];
			modified = true;
		}

		if (!user_config["matching"].contains("type") || (user_config["matching"]["type"].get<std::string>() != "exact" &&
			user_config["matching"]["type"].get<std::string>() != "prefix" &&
			user_config["matching"]["type"].get<std::string>() != "suffix")) {
			user_config["matching"]["type"] = default_config["matching"]["type"];
			modified = true;
		}
	}

	return modified;
}