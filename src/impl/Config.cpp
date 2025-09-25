#include "Config.h"

#include <iostream>
#include <filesystem>
#include <fstream>


Config::Config(const std::string& config_path) {
	// We only use this for testing purposes
	if (not config_path.empty())
		this->config_path = config_path;

	// If we haven't created a config file yet, or the user has deleted it, or they are pointing to a non-existent file, create a new config file
	if (!std::filesystem::exists(this->config_path)) {
		this->config = default_config;

		try {
			// Create the parent directory if it doesn't exist
			std::filesystem::path parent_path = std::filesystem::path(this->config_path).parent_path();
			if (!parent_path.empty() and !std::filesystem::exists(parent_path))
				std::filesystem::create_directories(parent_path);

		} catch (const std::exception& e) {
			std::cerr << "Error creating directories: " << e.what() << std::endl;
			return;
		}

		try {
			std::ofstream out_file(this->config_path);
			out_file << this->config.dump(4);
			out_file.close();
		} catch (const std::exception& e) {
			std::cerr << "Error writing config file: " << e.what() << std::endl;
			return;
		}

		return;
	}

	// Here, we can assume that the either our default config file exists in the default location or the user has provided a custom, valid path
	try {
		std::ifstream in_file(this->config_path);

		// On failure to open the file, use the default config
		if (!in_file.is_open()) {
			this->config = default_config;
			return;
		}

		// Create a new json object that we will read the config data into and modify if needed
		json user_config;
		in_file >> user_config;
		in_file.close();

		// Validate the config file
		if (validate_config(user_config)) {
			// If the config file is invalid, fix it with default values
			std::ofstream out_file(this->config_path);
			out_file << user_config.dump(4);
			out_file.close();
		}

		// Return the (maybe) modified user config that is valid
		this->config = user_config;

	} catch (const std::exception& e) {
		
		std::cerr << "Error reading config file: " << e.what() << ". Using default config." << std::endl;
		this->config = default_config;
		std::cout << this->config.dump(4) << std::endl;
		return;
	}
}

bool Config::validate_config(json& user_config) const {
	bool modified = false;

	// If "paths" key is missing, add it
	if (!user_config.contains("paths")) {
		user_config["paths"] = default_config["paths"];
		modified = true;
	} else {
		// Now check the sub-keys to see if they are present and valid
		if (!user_config["paths"].contains("init") or (!std::filesystem::exists(user_config["paths"]["init"].get<std::string>()))) {
			user_config["paths"]["init"] = default_config["paths"]["init"];
			modified = true;
		}

		if (!user_config["paths"].contains("db") or (!std::filesystem::exists(user_config["paths"]["db"].get<std::string>()))) {
			user_config["paths"]["db"] = default_config["paths"]["db"];
			modified = true;
		}
	}


	// If "matching" key is missing, add it
	if (!user_config.contains("matching")) {
		user_config["matching"] = default_config["matching"];
		modified = true;
	} else {
		// Now check the sub-keys to see if they are present and valid
		if (!user_config["matching"].contains("max_results") or (user_config["matching"]["max_results"].get<int>() <= 0)) {
			user_config["matching"]["max_results"] = default_config["matching"]["max_results"];
			modified = true;
		}

		if (!user_config["matching"].contains("max_history_size") or (user_config["matching"]["max_history_size"].get<int>() <= 0)) {
			user_config["matching"]["max_history_size"] = default_config["matching"]["max_history_size"];
			modified = true;
		}

		if (!user_config["matching"].contains("type") or (user_config["matching"]["type"].get<std::string>() != "exact" and
			user_config["matching"]["type"].get<std::string>() != "prefix" and
			user_config["matching"]["type"].get<std::string>() != "suffix" and
			user_config["matching"]["type"].get<std::string>() != "contains")) {
			user_config["matching"]["type"] = default_config["matching"]["type"];
			modified = true;
		}

		if (!user_config["matching"].contains("promotion_strategy") or (user_config["matching"]["promotion_strategy"].get<std::string>() != "recently_accessed" and
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
				if (type.key() != "prefix" and type.key() != "exact" and type.key() != "suffix" and type.key() != "contains") {
					user_config["matching"]["exclusions"][type.key()] = default_config["matching"]["exclusions"][type.key()];
					modified = true;
				}
				// If any of the values are not of type array, use the default for that type
				if (!type.value().is_array() and !type.value().is_null()) {
					user_config["matching"]["exclusions"][type.key()] = default_config["matching"]["exclusions"][type.key()];
					modified = true;
				}
			}
		}
	}

	return modified;
}

std::vector<ExclusionRule> Config::generate_exclusion_rules(const json& exclusions) const {
	std::vector<ExclusionRule> rules;

	// Generate the exclusion rules from the config file
	for (const auto& [excl_type, excl_patterns] : exclusions.items()) {
		// We expect the exclusion patterns to be an array of string/s
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