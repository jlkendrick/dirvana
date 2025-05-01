#include <gtest/gtest.h>

#include "Database.h"
#include "utils/TempConfigFile.hpp"

using namespace std;
using ConfigArgs = TempConfigFile::Args;

void unordered_check(const string& root, const vector<string>& completions, const vector<string>& expected) {
	EXPECT_EQ(completions.size(), expected.size());
	for (const auto& expected_path : expected) {
		EXPECT_NE(find(completions.begin(), completions.end(), root + expected_path), completions.end());
	}
}

class DatabaseTest : public ::testing::Test {
	protected:
		void SetUp() override {
			// Create a temporary config file for testing
			ConfigArgs args;
			temp_config_file = make_unique<TempConfigFile>(args);
			config = make_unique<Config>(temp_config_file->get_path());
		}
		void TearDown() override {
			// Clean up the temporary config file
			temp_config_file.reset();
		}
		unique_ptr<TempConfigFile> temp_config_file;
		unique_ptr<Config> config;
		unique_ptr<Database> db;
};

TEST_F(DatabaseTest, CreateDatabase) {
	// Test if the database is created successfully
	EXPECT_NO_THROW(db = make_unique<Database>(*config));
}

TEST_F(DatabaseTest, BuildDatabase) {
	// Test if the table is created successfully
	EXPECT_NO_THROW(db = make_unique<Database>(*config));
	EXPECT_NO_THROW(db->build());
}

TEST(Database, RefreshDatabase) {
	// Test if the database is refreshed successfully
	TempConfigFile temp_config{
		ConfigArgs{ 
			.exclusions = { 
				{ ExclusionType::Exact, "custom_rule_check" }
			} 
		}
	};
	Config config(temp_config.path);
	Database db(config);
	EXPECT_NO_THROW(db.build());

	// Change the exclusion rules to include/exclude directories
	config.set_exclusion_rules({
		{ ExclusionType::Prefix, "." },
		{ ExclusionType::Exact, config.get_init_path() + "/1/1/1/4" }
	});
	EXPECT_NO_THROW(db.refresh());
}

TEST_F(DatabaseTest, QueryDatabase) {
	// Test if the database can be queried successfully

	// Exact check
	EXPECT_NO_THROW(db = make_unique<Database>(*config));
	EXPECT_NO_THROW(db->build());
	unordered_check(config->get_init_path(), db->query("1"), {"/1", "/1/1", "/1/1/1"});

	config->set_exclusion_rules({});

	// Prefix check
	config->set_matching_type("prefix");
	EXPECT_NO_THROW(db->build());
	unordered_check(config->get_init_path(), db->query("."), {"/.1", "/custom_rule_check/.dot_check" }); 

	// Suffix check
	config->set_matching_type("suffix");
	EXPECT_NO_THROW(db->build());
	unordered_check(config->get_init_path(), db->query("eck"), {
		"/custom_rule_check",
		"/custom_rule_check/.dot_check",
		"/custom_rule_check/contains_check",
		"/custom_rule_check/exact_check",
		"/custom_rule_check/prefix_check",
		"/custom_rule_check/suffix_check"
	});

	// Contains check
	config->set_matching_type("contains");
	EXPECT_NO_THROW(db->build());
	unordered_check(config->get_init_path(), db->query("fix"), {
		"/custom_rule_check/prefix_check",
		"/custom_rule_check/suffix_check"
	});
}