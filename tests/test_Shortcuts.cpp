#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>

#include "Database.h"
#include "utils/TempConfigFile.hpp"

using namespace std;
using ConfigArgs = TempConfigFile::Args;

class ShortcutsTest : public ::testing::Test {
protected:
	void SetUp() override {
		ConfigArgs args;
		args.db_path = (filesystem::temp_directory_path() / "dirvana_shortcuts_test.db").string();
		filesystem::remove(args.db_path);
		temp_config = make_unique<TempConfigFile>(args);
		config = make_unique<Config>(temp_config->get_path());
		db = make_unique<Database>(*config);
		// No build() — shortcuts are independent of the filesystem scan
	}
	void TearDown() override {
		string db_path = config->get_db_path();
		db.reset();
		config.reset();
		temp_config.reset();
		filesystem::remove(db_path);
	}
	unique_ptr<TempConfigFile> temp_config;
	unique_ptr<Config> config;
	unique_ptr<Database> db;
};

// Directly exercises the lambda-return bug fix in ShortcutsTable::query()
TEST_F(ShortcutsTest, AddAndQuery) {
	db->get_shortcuts_table().add_shortcut("gs", "git status");
	auto results = db->get_shortcuts_table().query("gs");
	ASSERT_EQ(results.size(), 1u);
	EXPECT_EQ(results[0], "git status");
}

TEST_F(ShortcutsTest, QueryNonExistent) {
	auto results = db->get_shortcuts_table().query("nonexistent");
	EXPECT_TRUE(results.empty());
}

TEST_F(ShortcutsTest, DeleteShortcut) {
	db->get_shortcuts_table().add_shortcut("gs", "git status");
	db->get_shortcuts_table().delete_shortcut("gs");
	auto results = db->get_shortcuts_table().query("gs");
	EXPECT_TRUE(results.empty());
}

TEST_F(ShortcutsTest, SelectShortcutCommand) {
	db->get_shortcuts_table().add_shortcut("gs", "git status");
	EXPECT_EQ(db->get_shortcuts_table().select_shortcut_command("gs"), "git status");
}

TEST_F(ShortcutsTest, SelectNonExistentCommand) {
	EXPECT_EQ(db->get_shortcuts_table().select_shortcut_command("nope"), "");
}

TEST_F(ShortcutsTest, ListAllShortcuts) {
	db->get_shortcuts_table().add_shortcut("gs", "git status");
	db->get_shortcuts_table().add_shortcut("la", "ls -la");
	db->get_shortcuts_table().add_shortcut("g", "cd ~/Code");
	auto shortcuts = db->get_shortcuts_table().select_all_shortcuts();
	ASSERT_EQ(shortcuts.size(), 3u);
	EXPECT_NE(find(shortcuts.begin(), shortcuts.end(), "gs"), shortcuts.end());
	EXPECT_NE(find(shortcuts.begin(), shortcuts.end(), "la"), shortcuts.end());
	EXPECT_NE(find(shortcuts.begin(), shortcuts.end(), "g"), shortcuts.end());
}

TEST_F(ShortcutsTest, ListEmpty) {
	EXPECT_TRUE(db->get_shortcuts_table().select_all_shortcuts().empty());
}
