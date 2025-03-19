#include <gtest/gtest.h>

#include "DirectoryCompleter.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

struct TempConfigFile {
	struct Args {
		string cache_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/caches/test-cache.json";
		string init_path = "/Users/jameskendrick/Code/Projects/dirvana/tests/mockfs";
		int max_results = 10;
		string match_type = "exact";
		string promotion_strategy = "recently_accessed";
		vector<ExclusionRule> exclusions = {
			{ ExclusionType::Prefix, "."},
			{ ExclusionType::Exact, "custom_rule_check" },
		};
	};

	TempConfigFile(const Args& args) {
		path = "/Users/jameskendrick/Code/Projects/dirvana/tests/temp-config.json";
		json j;
		j["paths"]["cache"] = args.cache_path;
		j["paths"]["init"] = args.init_path;
		j["matching"]["max_results"] = args.max_results;
		j["matching"]["type"] = args.match_type;
		j["matching"]["promotion_strategy"] = args.promotion_strategy;
		j["matching"]["exclusions"] = TypeConversions::exclusion_rules_to_json(args.exclusions);

		ofstream file(path);
		if (file.is_open()) {
			file << j.dump(4);
			file.close();
		} else
			std::cerr << "Unable to open file for writing: " << path << std::endl;
	};

	~TempConfigFile() {
		if (std::filesystem::exists(path))
			std::filesystem::remove(path);
	};
	string path;
};



class RACDirectoryCompleter : public ::testing::TestWithParam<string> {
protected:
	void SetUp() override {
		string test_config = GetParam();
		completer = DirectoryCompleter(DCArgs{ .config_path = test_config });
	}
	
	DirectoryCompleter completer;
};

void check(const string& root, const vector<string>& completions, const vector<string>& expected) {
	EXPECT_EQ(completions.size(), expected.size());
	for (int i = 0; i < completions.size(); i++) {
		EXPECT_EQ(completions[i], root + expected[i]);
	}
}

TEST_F(RACDirectoryCompleter, Initialization) {
	vector<ExclusionRule> exclude = {
		{ ExclusionType::Prefix, "."},
		{ ExclusionType::Exact, "custom_rule_check" },
	};
	for (auto& rule : exclude)
		completer.add_exclusion_rule(rule);

	EXPECT_EQ(completer.get_size(), 10);
}


TEST_F(RACDirectoryCompleter, ExactCompletion) {
	vector<ExclusionRule> exclude = {
		{ ExclusionType::Prefix, "."},
    { ExclusionType::Exact, "custom_rule_check" },
  };
	string root = completer.get_config()["paths"]["init"].get<string>();

	auto completions = completer.get_matches("0");
	check(root, completions, {});

	completions = completer.get_matches("1");
	check(root, completions, {"/1", "/1/1", "/1/1/1"});

	completions = completer.get_matches("2");
	check(root, completions, {"/2", "/2/2"});

	completions = completer.get_matches("3");
	check(root, completions, {"/3"});

	completions = completer.get_matches("4");
	check(root, completions, {"/1/1/1/4", "/4", "/3/4", "/2/2/4"});

}

TEST(DirectoryCompleter, Access) {
	string test_config = "/Users/jameskendrick/Code/Projects/dirvana/tests/configs/base.json";
	vector<ExclusionRule> exclude = {
		{ ExclusionType::Prefix, "."},
    { ExclusionType::Exact, "custom_rule_check" },
  };
	DirectoryCompleter completer(DCArgs{ .config_path = test_config });
	string root = completer.get_config()["paths"]["init"].get<string>();

	completer.access(root + "/0");
	check(root, completer.get_matches("0"), {"/0"});

	completer.access(root + "/1/1");
	check(root, completer.get_matches("1"), {"/1/1", "/1", "/1/1/1"});

	completer.access(root + "/1/1/1");
	check(root, completer.get_matches("1"), {"/1/1/1", "/1/1", "/1"});

	completer.access(root + "/1/1/1");
	check(root, completer.get_matches("1"), {"/1/1/1", "/1/1", "/1"});
}

TEST(DirectoryCompleter, Exclusion) {
	string test_config = "/Users/jameskendrick/Code/Projects/dirvana/tests/configs/exclusion.json";
	vector<ExclusionRule> exclude = {
		{ ExclusionType::Prefix, "." },
	};
	DirectoryCompleter completer(DCArgs{ .config_path = test_config });
	EXPECT_EQ(completer.get_size(), 4);

	exclude.push_back({ ExclusionType::Exact, "exact_check" });
	DirectoryCompleter completer2(DCArgs{ .config_path = test_config });
	EXPECT_EQ(completer2.get_size(), 3);

	exclude.push_back({ ExclusionType::Contains, "ain" });
	DirectoryCompleter completer3(DCArgs{ .config_path = test_config });
	EXPECT_EQ(completer3.get_size(), 2);

	exclude.push_back({ ExclusionType::Suffix, "eck" });
	DirectoryCompleter completer4(DCArgs{ .config_path = test_config });
	EXPECT_EQ(completer4.get_size(), 0);
}

TEST(DirectoryCompleter, Refresh) {
	string test_config = "/Users/jameskendrick/Code/Projects/dirvana/tests/configs/base.json";

	// First create a completer that excludes a directory
	vector<ExclusionRule> exclude1 = {
		{ ExclusionType::Exact, "custom_rule_check" }
	};
	string cache_path = "test-refresh-cache.json";
	DirectoryCompleter original(DCArgs{ .config_path = test_config });
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
	DirectoryCompleter refreshed(DCArgs{ .build = false, .refresh = true, .config_path = test_config });

	// Verify that previously excluded directories are now included
	EXPECT_EQ(refreshed.get_size(), 14);
	
	// Verify specific directories are now accessible
	auto matches = refreshed.get_matches(".1");
	check(root, matches, {}); // Make sure the excluded directory is not included
	matches = refreshed.get_matches("custom_rule_check");
	check(root, matches, {"/custom_rule_check"}); // Make sure the previously excluded directory is now included

	refreshed.save();

	// Verify the order of the directories is preserved
	refreshed.access(root + "/2/2/4");
	refreshed.access(root + "/3/4");
	refreshed.access(root + "/4");
	vector<ExclusionRule> exclude3 = {
		{ ExclusionType::Exact, root + "/4" }
	};
	DirectoryCompleter refreshed2(DCArgs{ .build = false, .refresh = true, .config_path = test_config });
	auto refreshed2_matches = refreshed2.get_matches("4");
	check(root, refreshed2_matches, {"/3/4", "/2/2/4", "/1/1/1/4"});
}

TEST(DirectoryCompleter, SaveAndLoad) {
	// Create a DirectoryCompleter with known data
	string test_config = "/Users/jameskendrick/Code/Projects/dirvana/tests/configs/base.json";

	vector<ExclusionRule> exclude = {
		{ ExclusionType::Prefix, "."},
    { ExclusionType::Exact, "custom_rule_check" },
  };
	DirectoryCompleter original(DCArgs{ .config_path = test_config });
	string root = original.get_config()["paths"]["init"].get<string>();

	// Access some paths
	original.access(root + "/1/1");
	original.access(root + "/2/2");
	original.access(root + "/2/2/4");

	// Save the completer state
	original.save();

	// Create a new completer instance and load the saved state
	DirectoryCompleter loaded(DCArgs{ false, false, test_config });

	// Verify the loaded completer has the same matches
	auto loaded_matches_1 = loaded.get_matches("1");
	auto loaded_matches_2 = loaded.get_matches("2");
	auto loaded_matches_4 = loaded.get_matches("4");

	// Compare the results (reordering should persist)
	check(root, loaded_matches_1, {"/1/1", "/1", "/1/1/1"});
	check(root, loaded_matches_2, {"/2/2", "/2"});
	check(root, loaded_matches_4, {"/2/2/4", "/1/1/1/4", "/4", "/3/4"});

	// Verify that both completers have the same number of directories
	EXPECT_EQ(original.get_size(), loaded.get_size());
}