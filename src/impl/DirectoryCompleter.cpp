#include "DirectoryCompleter.h"

#include <memory>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;


DirectoryCompleter::DirectoryCompleter(const DCArgs& args) : test_mode(args.test_mode) {
	// If we were given a config path, use it (should only be used for testing, not available in the main program)
	if (!args.config_path.empty())
		this->config_path = args.config_path;
	// Load the config file and validate the contents
	this->config = load_config();
	// If we were given exclusion rules, use them
	this->exclusion_rules = generate_exclusion_rules(config["matching"]["exclusions"]);
	// Now we need to indicate the promotion strategy from the config for add()s to the PathMap (now path_map)
	this->strategy = TypeConversions::s_to_promotion_strategy(config["matching"]["promotion_strategy"].get<std::string>());
	
	// If we are building the cache, we add every directory in the root directory to the PathMap
	if (args.build) {
		for (const auto& [path, dir] : collect_directories())
			this->add(path, dir);
		
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
			add(path, dir);
		
		// Otherwise, remove it from the set of old directories
		else
			old_dirs.erase(path);
	}

	// Now, old_dirs contains the directories that were removed so remove them from the PathMap
	for (const std::string& dir : old_dirs)
		this->remove(dir);
}

void DirectoryCompleter::save() const {
	// First we need to serialize the path_map into a JSON object
	json mappings = json::array();
	for (const auto& [dir, cache] : path_map) {
		json entry;
		entry["key"] = dir;

		entry["entries"] = cache->serialize_entries();

		mappings.push_back(entry);
	}

	// Second, we need to keep track of the history of accesses for the purposes of match return order prioritization
	int max_history_size = config["matching"]["max_history_size"].get<int>();
	ordered_json history = access_history.serialize_entries(max_history_size);
	
	// Now create the final JSON object to save
	json j;
	j["mappings"] = mappings;
	j["history"] = history;

	// Save the cache to the file
	std::string cache_path = config["paths"]["cache"].get<std::string>();
	
	// Create parent directories if they don't exist
	try {
		fs::path dir_path = fs::path(cache_path).parent_path();
		if (!dir_path.empty() && !fs::exists(dir_path)) {
			fs::create_directories(dir_path);
		}
		
		std::ofstream file(cache_path);
		if (file.is_open()) {
			file << j.dump(4);
			file.close();
		} else {
			std::cerr << "Unable to open file for writing: " << cache_path << std::endl;
		}
	} catch (const fs::filesystem_error& e) {
		std::cerr << "Error creating directories: " << e.what() << std::endl;
	}
}

void DirectoryCompleter::load(std::unordered_set<std::string>& old_dirs) {
	std::string cache_path = config["paths"]["cache"].get<std::string>();
	
	// Check if the cache file exists for the promotion strategy we are using
	if (!fs::exists(cache_path)) {
		// If not, here we force a refresh with old_dirs empty so we essentially rebuild the cache
		refresh_directories(old_dirs);
		save(); // We also want to save the cache to the file for next time 
		return;
	}

	std::ifstream file(cache_path);
	if (file.is_open()) {
		json j;
		file >> j;
		file.close();

		json mappings = j["mappings"];
		for (const auto& item : mappings) {
			std::string key = item["key"];
			ordered_json entries = item["entries"];

			for (const auto& entry : entries) {
				this->add(entry, key);
				old_dirs.insert(entry["path"].get<std::string>());
			}
		}

		ordered_json history = j["history"];
		for (const auto& entry : history)
			access_history.add(entry);

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

std::vector<ExclusionRule> DirectoryCompleter::generate_exclusion_rules(const json& exclusions) const {
	std::vector<ExclusionRule> rules;
	for (const auto& [excl_type, excl_patterns] : exclusions.items()) {
		if (excl_patterns.is_array()) {
			for (const auto& value : excl_patterns) {
				rules.push_back(TypeConversions::s_to_exclusion_rule(excl_type, value.get<std::string>()));
			}
		} else {
			std::cerr << "Invalid exclusion value: " << excl_patterns.dump() << std::endl;
		}
	}

	return rules;
}

json DirectoryCompleter::load_config() const {
	// Check if the config file exists
	if (!fs::exists(config_path)) {
		// If not, create it with default values
		// std::cerr << "Config file not found. Creating default config file at: " << config_path << std::endl;

		// Create the parent directories if they don't exist
		try {
			fs::path dir_path = fs::path(config_path).parent_path();
			if (!dir_path.empty() && !fs::exists(dir_path)) {
				fs::create_directories(dir_path);
			}
		} catch (const fs::filesystem_error& e) {
			std::cerr << "Error creating directories: " << e.what() << std::endl;
		}
		
		// Create the default config
		std::ofstream out_file(config_path);
		out_file << default_config.dump(4);
		out_file.close();
		
		json user_config = default_config;
		// Remember we have to complete the cache path and since this sets the default, we use recently_accessed
		user_config["paths"]["cache"] = default_config["paths"]["cache"].get<std::string>() + "recently_accessed-cache.json";
		return user_config;
	}

	try {
		std::ifstream in_file(config_path);
		if (!in_file.is_open()) {
			// std::cerr << "Unable to open config file: " << config << std::endl;
			return default_config;
		}

		json user_config;
		in_file >> user_config;
		in_file.close();

		// Validate the config file
		if (validate_config(user_config)) {
			// std::cerr << "Config file invalid. Fixing it with default values." << std::endl;
			std::ofstream out_file(config_path);
			out_file << user_config.dump(4);
			out_file.close();
		}

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

		// if (!user_config["paths"].contains("cache") || (!fs::exists(user_config["paths"]["cache"].get<std::string>()))) {
		// 	user_config["paths"]["cache"] = default_config["paths"]["cache"];
		// 	modified = true;
		// }
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

		if (!user_config["matching"].contains("max_history_size") || (user_config["matching"]["max_history_size"].get<int>() <= 0)) {
			user_config["matching"]["max_history_size"] = default_config["matching"]["max_history_size"];
			modified = true;
		}

		if (!user_config["matching"].contains("type") || (user_config["matching"]["type"].get<std::string>() != "exact" &&
			user_config["matching"]["type"].get<std::string>() != "prefix" &&
			user_config["matching"]["type"].get<std::string>() != "suffix" &&
			user_config["matching"]["type"].get<std::string>() != "contains")) {
			user_config["matching"]["type"] = default_config["matching"]["type"];
			modified = true;
		}

		if (!user_config["matching"].contains("promotion_strategy") || (user_config["matching"]["promotion_strategy"].get<std::string>() != "recently_accessed" &&
			user_config["matching"]["promotion_strategy"].get<std::string>() != "frequency_based")) {
			user_config["matching"]["promotion_strategy"] = default_config["matching"]["promotion_strategy"];
			modified = true;
		}

		
		if (!user_config["matching"].contains("exclusions")) {
			user_config["matching"]["exclusions"] = default_config["matching"]["exclusions"];
			modified = true;
		} else {
			// If any of the exclusion types are not valid, use the default for that type
			for (auto& type : user_config["matching"]["exclusions"].items()) {
				if (type.key() != "prefix" && type.key() != "exact" && type.key() != "suffix" && type.key() != "contains") {
					user_config["matching"]["exclusions"][type.key()] = default_config["matching"]["exclusions"][type.key()];
					modified = true;
				}
				// If any of the values are not of type array, use the default for that type
				if (!type.value().is_array() && !type.value().is_null()) {
					user_config["matching"]["exclusions"][type.key()] = default_config["matching"]["exclusions"][type.key()];
					modified = true;
				}
			}
		}
	}

	// Now that the promotion strategy is guaranteed to be defined, we can define the cache path or, if in testing mode, use the one given (in user_config already)
	if (!test_mode) {
		if (user_config["matching"]["promotion_strategy"].get<std::string>() == "recently_accessed")
			user_config["paths"]["cache"] = default_config["paths"]["cache"].get<std::string>() + "recently_accessed-cache.json";
		else
			user_config["paths"]["cache"] = default_config["paths"]["cache"].get<std::string>() + "frequency_based-cache.json";
	}

	return modified;
}