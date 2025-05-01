#ifndef CONFIG_H
#define CONFIG_H

#include "Helpers.h"
#include "Types.h"

#include <json.hpp>

using json = nlohmann::json;


// Config class loads, validates, and provides access to the configuration data
class Config {
public:
	Config(const std::string& config_path);

	// Getters for the configuration data
	const json& get_config() const { return config; }
	std::string get_init_path() const { return config["paths"]["init"].get<std::string>(); }
	std::string get_db_path() const { return config["paths"]["db"].get<std::string>(); }
	std::string get_history_path() const { return config["paths"]["history"].get<std::string>(); }
	int get_max_results() const { return config["matching"]["max_results"].get<int>(); }
	int get_max_history_size() const { return config["matching"]["max_history_size"].get<int>(); }
	std::string get_matching_type() const { return config["matching"]["type"].get<std::string>(); }
	PromotionStrategy get_promotion_strategy() const {
		return TypeConversions::s_to_promotion_strategy(config["matching"]["promotion_strategy"].get<std::string>());
	}
	const std::vector<ExclusionRule>& get_exclusion_rules() const { return exclusion_rules;}

	
private:
	std::string config_path;
	json config;
	json default_config = {
		{"paths", {
			{"init", std::getenv("HOME") + std::string("/")},
			{"db", std::getenv("HOME") + std::string("/.cache/dirvana/dirvana.db")},
			{"history", std::getenv("HOME") + std::string("/.cache/dirvana/history.json")}
		}},
		{"matching", {
			{"max_results", 10},
			{"max_history_size", 100},
			{"type", "exact"},
			{"promotion_strategy", "recently_accessed"},
			{"exclusions", {
				{"prefix", {"."}},
				{"exact", {"node_modules", "browser_components", "dist", "out", "target", "tmp", "temp", "cache", "venv", "env", "obj", "pkg", "bin"}},
				{"suffix", {"sdk", "Library"}},
				{"contains", {"release"}}
				}
			}
		}}
	};
	std::vector<ExclusionRule> exclusion_rules;

	std::vector<ExclusionRule> generate_exclusion_rules(const json& exclusions) const;
	bool validate_config(json& user_config) const;
};

#endif // CONFIG_H