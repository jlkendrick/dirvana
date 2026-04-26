#include <gtest/gtest.h>

#include <filesystem>

#include "Database.h"
#include "utils/TempConfigFile.hpp"

using namespace std;
using ConfigArgs = TempConfigFile::Args;

void ordered_check(const string& root, const vector<string>& completions, const vector<string>& expected) {
	EXPECT_EQ(completions.size(), expected.size());
	for (unsigned int i = 0; i < completions.size(); i++) {
		EXPECT_EQ(completions[i], root + expected[i]);
	}
}

void unordered_check(const string& root, const vector<string>& completions, const vector<string>& expected) {
	EXPECT_EQ(completions.size(), expected.size());
	for (const auto& expected_path : expected) {
		EXPECT_NE(find(completions.begin(), completions.end(), root + expected_path), completions.end());
	}
}

class DatabaseTest : public ::testing::Test {
	protected:
		void SetUp() override {
			ConfigArgs args;
			filesystem::remove(args.db_path);
			temp_config_file = make_unique<TempConfigFile>(args);
			config = make_unique<Config>(temp_config_file->get_path());
		}
		void TearDown() override {
			string db_path = config->get_db_path();
			db.reset();
			config.reset();
			temp_config_file.reset();
			filesystem::remove(db_path);
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
	EXPECT_NO_THROW(db->build(config->get_init_path()));
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
	EXPECT_NO_THROW(db.build(config.get_init_path()));

	// Change the exclusion rules to include/exclude directories
	config.set_exclusion_rules({
		{ ExclusionType::Prefix, "." },
		{ ExclusionType::Exact, config.get_init_path() + "/1/1/1/4" }
	});
	EXPECT_NO_THROW(db.refresh(config.get_init_path()));
}

TEST_F(DatabaseTest, QueryDatabase) {
	// Test if the database can be queried successfully

	// Exact check
	EXPECT_NO_THROW(db = make_unique<Database>(*config));
	EXPECT_NO_THROW(db->build(config->get_init_path()));
	unordered_check(config->get_init_path(), db->get_paths_table().query("1"), {"/1", "/1/1", "/1/1/1"});

	config->set_exclusion_rules({});

	// Prefix check
	config->set_matching_type("prefix");
	EXPECT_NO_THROW(db->build(config->get_init_path()));
	unordered_check(config->get_init_path(), db->get_paths_table().query("."), {"/.1", "/custom_rule_check/.dot_check" });

	// Suffix check
	config->set_matching_type("suffix");
	EXPECT_NO_THROW(db->build(config->get_init_path()));
	unordered_check(config->get_init_path(), db->get_paths_table().query("eck"), {
		"/custom_rule_check",
		"/custom_rule_check/.dot_check",
		"/custom_rule_check/contains_check",
		"/custom_rule_check/exact_check",
		"/custom_rule_check/prefix_check",
		"/custom_rule_check/suffix_check"
	});

	// Contains check
	config->set_matching_type("contains");
	EXPECT_NO_THROW(db->build(config->get_init_path()));
	unordered_check(config->get_init_path(), db->get_paths_table().query("fix"), {
		"/custom_rule_check/prefix_check",
		"/custom_rule_check/suffix_check"
	});
}

TEST(Database, FrequencyBasedPromotion) {
	TempConfigFile temp_config{
		ConfigArgs{
			.match_type = "exact",
			.promotion_strategy = "frequency_based",
			.exclusions = { { ExclusionType::Prefix, "." }, { ExclusionType::Exact, "custom_rule_check" } }
		}
	};
	Config config(temp_config.path);
	Database db(config);
	db.build(config.get_init_path());

	// Access /1 once, /1/1 three times — frequency should drive ranking
	db.get_paths_table().access(config.get_init_path() + "/1");
	db.get_paths_table().access(config.get_init_path() + "/1/1");
	db.get_paths_table().access(config.get_init_path() + "/1/1");
	db.get_paths_table().access(config.get_init_path() + "/1/1");

	auto results = db.get_paths_table().query("1");
	ASSERT_GE(results.size(), 2u);
	EXPECT_EQ(results[0], config.get_init_path() + "/1/1");
	EXPECT_EQ(results[1], config.get_init_path() + "/1");
}

TEST_F(DatabaseTest, AccessDatabase) {
	// Test if the database can be accessed and updated successfully

	// Recently accessed check
	EXPECT_NO_THROW(db = make_unique<Database>(*config));
	EXPECT_NO_THROW(db->build(config->get_init_path()));

	EXPECT_NO_THROW(db->get_paths_table().access(config->get_init_path() + "/1/1"));
	EXPECT_NO_THROW(db->get_paths_table().access(config->get_init_path() + "/1/1/1"));
	ordered_check(config->get_init_path(), db->get_paths_table().query("1"), {"/1/1/1", "/1/1", "/1"});

	EXPECT_NO_THROW(db->get_paths_table().access(config->get_init_path() + "/1/1"));
	ordered_check(config->get_init_path(), db->get_paths_table().query("1"), {"/1/1", "/1/1/1", "/1"});

	EXPECT_NO_THROW(db->get_paths_table().access(config->get_init_path() + "/1"));
	ordered_check(config->get_init_path(), db->get_paths_table().query("1"), {"/1", "/1/1", "/1/1/1"});
}