#include <gtest/gtest.h>

#include "Database.h"
#include "utils/TempConfigFile.hpp"

using namespace std;
using ConfigArgs = TempConfigFile::Args;


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