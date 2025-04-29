#ifndef TEMPCONFIGFILE_H
#define TEMPCONFIGFILE_H

#include "Types.h"
#include "Helpers.h"
#include <json.hpp>

#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <set>

using json = nlohmann::json;

struct TempConfigFile {
	struct Args {
		std::string cache_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/caches/test-cache.json";
		std::string init_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs";
		int max_results = 10;
		int max_history_size = 100;
		std::string match_type = "exact";
		std::string promotion_strategy = "recently_accessed";
		std::vector<ExclusionRule> exclusions = {
			{ ExclusionType::Prefix, "."},
			{ ExclusionType::Exact, "custom_rule_check" },
		};

		std::set<std::string> forget;
		bool should_forget(const std::string& field) const {
			auto it = forget.find(field);
			if (it != forget.end())
				return true;
			return false;
		}
	};

	TempConfigFile(const Args& args) {
		json config;
		if (!args.should_forget("cache_path")) config["paths"]["cache"] = args.cache_path;
		if (!args.should_forget("init_path")) config["paths"]["init"] = args.init_path;
		if (!args.should_forget("max_results")) config["matching"]["max_results"] = args.max_results;
		if (!args.should_forget("max_history_size")) config["matching"]["max_history_size"] = args.max_history_size;
		if (!args.should_forget("match_type")) config["matching"]["type"] = args.match_type;
		if (!args.should_forget("promotion_strategy")) config["matching"]["promotion_strategy"] = args.promotion_strategy;
		if (!args.should_forget("exclusions")) config["matching"]["exclusions"] = TypeConversions::exclusion_rules_to_json(args.exclusions);

		save_to_file(config);
	};
	~TempConfigFile() {
		if (std::filesystem::exists(path))
			std::filesystem::remove(path);
	}; // Delete the temporary file when the object is destroyed

	// Save the config to a temporary file
	void save_to_file(const json& config) const {
		std::ofstream out_file(path);
		out_file << config.dump(4);
		out_file.close();
	};

	std::string path = "/Users/jameskendrick/Code/Projects/dirvana/tests/configs/temp-config.json";
};

#endif // TEMPCONFIGFILE_H