#include <gtest/gtest.h>

#include "DirectoryCompleter.h"
#include "utils/TempConfigFile.hpp"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using ConfigArgs = TempConfigFile::Args;


void ordered_check(const string& root, const vector<string>& completions, const vector<string>& expected) {
	EXPECT_EQ(completions.size(), expected.size());
	for (int i = 0; i < completions.size(); i++) {
		EXPECT_EQ(completions[i], root + expected[i]);
	}
}

void unordered_check(const string& root, const vector<string>& completions, const vector<string>& expected) {
	EXPECT_EQ(completions.size(), expected.size());
	for (const auto& expected_path : expected) {
		EXPECT_NE(find(completions.begin(), completions.end(), root + expected_path), completions.end());
	}
}


TEST(DirectoryCompleter, Initialization) {
	TempConfigFile temp_config{ConfigArgs()};
	DirectoryCompleter completer(DCArgs{ .config_path = temp_config.path, .test_mode = true });
	
	EXPECT_EQ(completer.get_size(), 10);
}

TEST(DirectoryCompleter, ConfigParsing) {
	// Test the default test config
	TempConfigFile temp_config1{ConfigArgs()};
	DirectoryCompleter completer1(DCArgs{ .config_path = temp_config1.path, .test_mode = true });

	json config1 = completer1.get_config();
	EXPECT_EQ(config1["paths"]["init"].get<string>(), "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs");
	EXPECT_EQ(config1["paths"]["cache"].get<string>(), "/Users/jameskendrick/Code/Projects/dirvana/tests/caches/test-cache.json");
	EXPECT_EQ(config1["matching"]["max_results"].get<int>(), 10);
	EXPECT_EQ(config1["matching"]["max_history_size"].get<int>(), 100);
	EXPECT_EQ(config1["matching"]["type"].get<string>(), "exact");
	EXPECT_EQ(config1["matching"]["promotion_strategy"].get<string>(), "recently_accessed");
	EXPECT_EQ(config1["matching"]["exclusions"]["prefix"].get<vector<string>>(), vector<string>{"."});
	EXPECT_EQ(config1["matching"]["exclusions"]["exact"].get<vector<string>>(), vector<string>{"custom_rule_check"});
	
	// Test a custom config
	TempConfigFile temp_config2{ConfigArgs{
		.cache_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/caches/frequency_based-cache.json",
		.init_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs",
		.max_results = 5,
		.max_history_size = 75,
		.match_type = "prefix",
		.promotion_strategy = "frequency_based",
		.exclusions = {
			{ ExclusionType::Exact, "a" },
			{ ExclusionType::Exact, "b" },
			{ ExclusionType::Prefix, "c" },
			{ ExclusionType::Prefix, "d" },
			{ ExclusionType::Suffix, "e" },
			{ ExclusionType::Suffix, "f" },
			{ ExclusionType::Contains, "g" },
			{ ExclusionType::Contains, "h" }
	}}};
	DirectoryCompleter completer2(DCArgs{ .config_path = temp_config2.path, .test_mode = true });

	json config2 = completer2.get_config();
	EXPECT_EQ(config2["paths"]["init"].get<string>(), "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs");
	EXPECT_EQ(config2["paths"]["cache"].get<string>(), "/Users/jameskendrick/Code/Projects/dirvana/tests/caches/frequency_based-cache.json");
	EXPECT_EQ(config2["matching"]["max_results"].get<int>(), 5);
	EXPECT_EQ(config2["matching"]["max_history_size"].get<int>(), 75);
	EXPECT_EQ(config2["matching"]["type"].get<string>(), "prefix");
	EXPECT_EQ(config2["matching"]["promotion_strategy"].get<string>(), "frequency_based");
	EXPECT_EQ(config2["matching"]["exclusions"]["exact"].get<vector<string>>(), (vector<string>{"a", "b"}));
	EXPECT_EQ(config2["matching"]["exclusions"]["prefix"].get<vector<string>>(), (vector<string>{"c", "d"}));
	EXPECT_EQ(config2["matching"]["exclusions"]["suffix"].get<vector<string>>(), (vector<string>{"e", "f"}));
	EXPECT_EQ(config2["matching"]["exclusions"]["contains"].get<vector<string>>(), (vector<string>{"g", "h"}));

	// Test a faulty config
	TempConfigFile temp_config3{ConfigArgs{
		.init_path = "/something/that/does/not/exist",
		.max_results = -1,
		.max_history_size = -100,
		.match_type = "not_a_valid_type",
		.promotion_strategy = "not_a_valid_strategy",
		.forget = {"exclusions"}
	}};
	DirectoryCompleter completer3(DCArgs{ .build = false, .config_path = temp_config3.path, .test_mode = true });

	json config3 = completer3.get_config();
	EXPECT_EQ(config3["paths"]["init"].get<string>(), "/Users/jameskendrick/");
	EXPECT_EQ(config3["paths"]["cache"].get<string>(), "/Users/jameskendrick/Code/Projects/dirvana/tests/caches/test-cache.json");
	EXPECT_EQ(config3["matching"]["max_results"].get<int>(), 10);
	EXPECT_EQ(config3["matching"]["max_history_size"].get<int>(), 100);
	EXPECT_EQ(config3["matching"]["type"].get<string>(), "exact");
	EXPECT_EQ(config3["matching"]["promotion_strategy"].get<string>(), "recently_accessed");
	EXPECT_EQ(config3["matching"]["exclusions"]["prefix"].get<vector<string>>(), vector<string>{"."});
	EXPECT_EQ(config3["matching"]["exclusions"]["exact"].get<vector<string>>(), (vector<string>{"node_modules", "browser_components", "dist", "out", "target", "tmp", "temp", "cache", "venv", "env", "obj", "pkg", "bin"}));
	EXPECT_EQ(config3["matching"]["exclusions"]["suffix"].get<vector<string>>(), (vector<string>{"sdk", "Library"}));
	EXPECT_EQ(config3["matching"]["exclusions"]["contains"].get<vector<string>>(), (vector<string>{"release"}));
}

TEST(DirectoryCompleter, ExactCompletion) {
	TempConfigFile temp_config{ConfigArgs()};
	DirectoryCompleter completer(DCArgs{ .config_path = temp_config.path, .test_mode = true });
	string root = completer.get_config()["paths"]["init"].get<string>();

	auto completions = completer.get_matches("0");
	ordered_check(root, completions, {});

	completions = completer.get_matches("1");
	ordered_check(root, completions, {"/1", "/1/1", "/1/1/1"});

	completions = completer.get_matches("2");
	ordered_check(root, completions, {"/2", "/2/2"});

	completions = completer.get_matches("3");
	ordered_check(root, completions, {"/3"});

	completions = completer.get_matches("4");
	ordered_check(root, completions, {"/1/1/1/4", "/4", "/3/4", "/2/2/4"});
}

TEST(DirectoryCompleter, PrefixCompletion) {
	TempConfigFile temp_config{ConfigArgs{ .max_results = 5, .match_type = "prefix" }};
	DirectoryCompleter completer(DCArgs{ .config_path = temp_config.path, .test_mode = true });
	string root = completer.get_config()["paths"]["init"].get<string>();
	
	// use .access() method to add new directories to the cache
	completer.access(root + "/word1");
	completer.access(root + "/intermediate/word1");
	completer.access(root + "/word2");
	completer.access(root + "/intermediate/word2");
	completer.access(root + "/word3");
	completer.access(root + "/word4");
	completer.access(root + "/word5");
	completer.access(root + "/notword1");
	completer.access(root + "/notword2");
	completer.access(root + "/notword3");
	completer.access(root + "/intermediate/notword1");

	auto completions = completer.get_matches("word");
	unordered_check(root, completions, {"/word1", "/word1", "/word2", "/word2", "/word3"});

	completions = completer.get_matches("no");
	unordered_check(root, completions, {"/notword1", "/notword2", "/notword3", "/intermediate/notword1"});

	completions = completer.get_matches("intermediate");
	ordered_check(root, completions, {});
}

TEST(DirectoryCompleter, SuffixCompletion) {
	TempConfigFile temp_config{ConfigArgs{ .max_results = 5, .match_type = "suffix" }};
	DirectoryCompleter completer(DCArgs{ .config_path = temp_config.path, .test_mode = true });
	string root = completer.get_config()["paths"]["init"].get<string>();

	completer.access(root + "/jubilation");
	completer.access(root + "/celebration");
	completer.access(root + "/imagination");
	completer.access(root + "/intermediate/jubilation");
	completer.access(root + "/salutation");
	completer.access(root + "/amalgamation");

	auto completions = completer.get_matches("tion");
	unordered_check(root, completions, {"/jubilation", "/celebration", "/imagination", "/jubilation", "/salutation"});

	completions = completer.get_matches("lation");
	unordered_check(root, completions, {"/jubilation", "/intermediate/jubilation"});

	completions = completer.get_matches("moisture");
	ordered_check(root, completions, {});
}

TEST(DirectoryCompleter, ContainsCompletion) {
	TempConfigFile temp_config{ConfigArgs{ .max_results = 100, .match_type = "contains" }};
	DirectoryCompleter completer(DCArgs{ .config_path = temp_config.path, .test_mode = true });
	string root = completer.get_config()["paths"]["init"].get<string>();

	completer.access(root + "/eating");
	completer.access(root + "/intermediate/eating");
	completer.access(root + "/plea");
	completer.access(root + "/intermediate/plea");
	completer.access(root + "/dealer");
	completer.access(root + "/intermediate/dealer");
	completer.access(root + "/toenail");
	completer.access(root + "/intermediate/toenail");

	// This should not do anything with regard to the return order since it does not change the underlying index
	// It will, however, affect the order when we load and save the cache
	completer.access(root + "/intermediate/plea");
	completer.access(root + "/intermediate/plea");

	auto completions = completer.get_matches("ea");
	unordered_check(root, completions, {"/eating", "/intermediate/eating", "/plea", "/intermediate/plea", "/dealer", "/intermediate/dealer"});

	completions = completer.get_matches("oenai");
	unordered_check(root, completions, {"/toenail", "/intermediate/toenail"});

	completions = completer.get_matches("xxxxxx");
	ordered_check(root, completions, {});
}

TEST(DirectoryCompleter, Access) {
	TempConfigFile temp_config{ConfigArgs()};
	DirectoryCompleter completer(DCArgs{ .config_path = temp_config.path, .test_mode = true });
	string root = completer.get_config()["paths"]["init"].get<string>();

	completer.access(root + "/0");
	ordered_check(root, completer.get_matches("0"), {"/0"});

	completer.access(root + "/1/1");
	ordered_check(root, completer.get_matches("1"), {"/1/1", "/1", "/1/1/1"});

	completer.access(root + "/1/1/1");
	ordered_check(root, completer.get_matches("1"), {"/1/1/1", "/1/1", "/1"});

	completer.access(root + "/1/1/1");
	ordered_check(root, completer.get_matches("1"), {"/1/1/1", "/1/1", "/1"});
}

TEST(DirectoryCompleter, Exclusion) {
	vector<ExclusionRule> exclude = {
		{ ExclusionType::Prefix, "."},
	};
	TempConfigFile temp_config1{ConfigArgs{ .init_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs/custom_rule_check", .exclusions = exclude }};
	DirectoryCompleter completer(DCArgs{ .config_path = temp_config1.path, .test_mode = true });
	EXPECT_EQ(completer.get_size(), 4);

	exclude.push_back({ ExclusionType::Exact, "exact_check" });
	TempConfigFile temp_config2{ConfigArgs{ .init_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs/custom_rule_check", .exclusions = exclude }};
	DirectoryCompleter completer2(DCArgs{ .config_path = temp_config2.path, .test_mode = true });
	EXPECT_EQ(completer2.get_size(), 3);

	exclude.push_back({ ExclusionType::Contains, "ain" });
	TempConfigFile temp_config3{ConfigArgs{ .init_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs/custom_rule_check", .exclusions = exclude }};
	DirectoryCompleter completer3(DCArgs{ .config_path = temp_config3.path, .test_mode = true });
	EXPECT_EQ(completer3.get_size(), 2);

	exclude.push_back({ ExclusionType::Suffix, "eck" });
	TempConfigFile temp_config4{ConfigArgs{ .init_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs/custom_rule_check", .exclusions = exclude }};
	DirectoryCompleter completer4(DCArgs{ .config_path = temp_config4.path, .test_mode = true });
	EXPECT_EQ(completer4.get_size(), 0);
}

TEST(DirectoryCompleter, Refresh) {
	// First create a completer that excludes a directory
	vector<ExclusionRule> exclude1 = {
		{ ExclusionType::Exact, "custom_rule_check" }
	};
	TempConfigFile temp_config{ConfigArgs{ .exclusions = exclude1 }}; 
	DirectoryCompleter original(DCArgs{ .config_path = temp_config.path, .test_mode = true });
	string root = original.get_config()["paths"]["init"].get<string>();

	// Verify initial state with strict exclusion
	EXPECT_EQ(original.get_size(), 11);

	// Save the current state
	original.save();

	// Create a new completer that includes the previously excluded directory, but excludes another
	vector<ExclusionRule> exclude2 = {
		{ ExclusionType::Prefix, "." },
		{ ExclusionType::Exact, root + "/1/1/1/4" }
	};
	TempConfigFile temp_config2{ConfigArgs{ .exclusions = exclude2 }};
	DirectoryCompleter refreshed(DCArgs{ .build = false, .refresh = true, .config_path = temp_config2.path, .test_mode = true });

	// Verify that previously excluded directories are now included
	EXPECT_EQ(refreshed.get_size(), 14);
	
	// Verify specific directories are now accessible
	auto matches = refreshed.get_matches(".1");
	ordered_check(root, matches, {}); // Make sure the excluded directory is not included
	matches = refreshed.get_matches("custom_rule_check");
	ordered_check(root, matches, {"/custom_rule_check"}); // Make sure the previously excluded directory is now included

	refreshed.save();

	// Verify the order of the directories is preserved
	refreshed.access(root + "/2/2/4");
	refreshed.access(root + "/3/4");
	refreshed.access(root + "/4");
	vector<ExclusionRule> exclude3 = {
		{ ExclusionType::Exact, root + "/4" }
	};
	TempConfigFile temp_config3{ConfigArgs{ .exclusions = exclude3 }};
	DirectoryCompleter refreshed2(DCArgs{ .build = false, .refresh = true, .config_path = temp_config3.path, .test_mode = true });
	auto refreshed2_matches = refreshed2.get_matches("4");
	ordered_check(root, refreshed2_matches, {"/3/4", "/2/2/4", "/1/1/1/4"});
}

TEST(DirectoryCompleter, SaveAndLoad) {
	// Create a DirectoryCompleter with known data
	TempConfigFile test_config{ConfigArgs{ .promotion_strategy = "frequency_based" }};
	DirectoryCompleter original(DCArgs{ .config_path = test_config.path, .test_mode = true });
	string root = original.get_config()["paths"]["init"].get<string>();

	// Access some paths
	original.access(root + "/1/1");
	original.access(root + "/2/2");
	original.access(root + "/2/2/4");

	// Save the completer state
	original.save();

	// Create a new completer instance and load the saved state
	DirectoryCompleter loaded(DCArgs{ false, false, test_config.path, .test_mode = true });

	// Verify the loaded completer has the same matches
	auto loaded_matches_1 = loaded.get_matches("1");
	auto loaded_matches_2 = loaded.get_matches("2");
	auto loaded_matches_4 = loaded.get_matches("4");

	// Compare the results (reordering should persist)
	ordered_check(root, loaded_matches_1, {"/1/1", "/1", "/1/1/1"});
	ordered_check(root, loaded_matches_2, {"/2/2", "/2"});
	ordered_check(root, loaded_matches_4, {"/2/2/4", "/1/1/1/4", "/4", "/3/4"});

	// Verify that both completers have the same number of directories
	EXPECT_EQ(original.get_size(), loaded.get_size());
}

TEST(DirectoryCompleter, HistoryPrioritization) {
	TempConfigFile temp_config{ConfigArgs{ .promotion_strategy = "recently_accessed", .match_type= "contains" }};
	DirectoryCompleter completer(DCArgs{ .config_path = temp_config.path, .test_mode = true });
	string root = completer.get_config()["paths"]["init"].get<string>();

	completer.access(root + "/1/1/1");
	completer.access(root + "/2/2");
	completer.access(root + "/3");
	
	vector<string> history = completer.get_history();
	EXPECT_EQ(history.size(), 3);
	EXPECT_EQ(1, 1);
}