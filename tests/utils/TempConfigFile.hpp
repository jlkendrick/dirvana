#ifndef TEMPCONFIGFILE_H
#define TEMPCONFIGFILE_H

#include "Types.h"
#include "Helpers.h"
#include "nlohmann/json.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

using json = nlohmann::json;

struct TempConfigFile {
	struct Args {
		std::string cache_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/caches/test-cache.json";
		std::string init_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs";
		int max_results = 10;
		std::string match_type = "exact";
		std::string promotion_strategy = "recently_accessed";
		std::vector<ExclusionRule> exclusions = {
			{ ExclusionType::Prefix, "."},
			{ ExclusionType::Exact, "custom_rule_check" },
		};
	};

	TempConfigFile(const Args& args) {
		json config;
		config["paths"]["cache"] = args.cache_path;
		config["paths"]["init"] = args.init_path;
		config["matching"]["max_results"] = args.max_results;
		config["matching"]["type"] = args.match_type;
		config["matching"]["promotion_strategy"] = args.promotion_strategy;
		config["matching"]["exclusions"] = TypeConversions::exclusion_rules_to_json(args.exclusions);

		save(config);
	};
	~TempConfigFile() {
		if (std::filesystem::exists(path))
			std::filesystem::remove(path);
	}; // Delete the temporary file when the object is destroyed

	// Save the config to a temporary file
	void save(const json& config) const {
		std::ofstream out_file(path);
		out_file << config.dump(4);
		out_file.close();
	};

	std::string path = "/Users/jameskendrick/Code/Projects/dirvana/tests/configs/temp-config.json";
};

#endif // TEMPCONFIGFILE_H