#include <gtest/gtest.h>

#include <filesystem>

#include "Database.h"
#include "Handler.h"
#include "utils/TempConfigFile.hpp"

using namespace std;
using ConfigArgs = TempConfigFile::Args;

class HandlerTest : public ::testing::Test {
protected:
	void SetUp() override {
		ConfigArgs args;
		args.db_path = (filesystem::temp_directory_path() / "dirvana_handler_test.db").string();
		filesystem::remove(args.db_path);
		temp_config = make_unique<TempConfigFile>(args);
		config = make_unique<Config>(temp_config->get_path());
		db = make_unique<Database>(*config);
		db->build(config->get_init_path());
		handler = make_unique<Handler>(*db);
	}
	void TearDown() override {
		string db_path = config->get_db_path();
		handler.reset();
		db.reset();
		config.reset();
		temp_config.reset();
		filesystem::remove(db_path);
	}

	// Runs handle_enter and returns {return_code, stdout_output}
	pair<int, string> run_enter(vector<string> commands, vector<Flag> flags = {}) {
		testing::internal::CaptureStdout();
		testing::internal::CaptureStderr();
		int ret = handler->handle_enter(commands, flags);
		testing::internal::GetCapturedStderr();
		return {ret, testing::internal::GetCapturedStdout()};
	}

	unique_ptr<TempConfigFile> temp_config;
	unique_ptr<Config> config;
	unique_ptr<Database> db;
	unique_ptr<Handler> handler;
};

// ---- Navigation ----

TEST_F(HandlerTest, EnterNoArgs) {
	auto [ret, output] = run_enter({}, {});
	EXPECT_EQ(ret, 0);
	EXPECT_EQ(output, "cd ~\n");
}

TEST_F(HandlerTest, EnterPartialPath) {
	string mockfs = config->get_init_path();
	auto [ret, output] = run_enter({"1"});
	EXPECT_EQ(ret, 0);
	EXPECT_TRUE(output.starts_with("cd " + mockfs));
	EXPECT_TRUE(output.ends_with("/1\n"));
}

TEST_F(HandlerTest, EnterFullAbsolutePath) {
	auto [ret, output] = run_enter({"/tmp"});
	EXPECT_EQ(ret, 0);
	EXPECT_EQ(output, "cd /tmp\n");
}

TEST_F(HandlerTest, EnterUnknownPartial) {
	auto [ret, output] = run_enter({"zzz_no_match"});
	EXPECT_EQ(ret, 0);
	EXPECT_EQ(output, "cd zzz_no_match\n");
}

TEST_F(HandlerTest, EnterVersionFlag) {
	auto [ret, output] = run_enter({}, {{"", "version", ""}});
	EXPECT_EQ(ret, 0);
	EXPECT_EQ(output, "echo Dirvana version 1.0.1\n");
}

// Shortcut takes priority over path lookup; no last token so command gets a trailing space
TEST_F(HandlerTest, EnterShortcutResolution) {
	db->get_shortcuts_table().add_shortcut("gs", "git status");
	auto [ret, output] = run_enter({"gs"});
	EXPECT_EQ(ret, 0);
	EXPECT_EQ(output, "git status \n");
}

// ---- Subcommands ----

TEST_F(HandlerTest, AddShortcut) {
	auto [ret, output] = run_enter({"add", "gs", "git status"});
	EXPECT_EQ(ret, 0);
	EXPECT_NE(output.find("echo Shortcut"), string::npos);
	// Verify the shortcut was persisted to the DB
	auto results = db->get_shortcuts_table().query("gs");
	ASSERT_EQ(results.size(), 1u);
	EXPECT_EQ(results[0], "git status");
}

TEST_F(HandlerTest, AddShortcutTooFewArgs) {
	auto [ret, output] = run_enter({"add", "gs"});
	EXPECT_EQ(ret, 1);
}

TEST_F(HandlerTest, DeleteShortcut) {
	db->get_shortcuts_table().add_shortcut("gs", "git status");
	auto [ret, output] = run_enter({"delete", "gs"});
	EXPECT_EQ(ret, 0);
	EXPECT_TRUE(db->get_shortcuts_table().query("gs").empty());
}

TEST_F(HandlerTest, DeleteShortcutWrongArgCount) {
	auto [ret, output] = run_enter({"delete"});
	EXPECT_EQ(ret, 1);
}

TEST_F(HandlerTest, ListShortcuts) {
	db->get_shortcuts_table().add_shortcut("gs", "git status");
	db->get_shortcuts_table().add_shortcut("la", "ls -la");
	auto [ret, output] = run_enter({"list"});
	EXPECT_EQ(ret, 0);
	EXPECT_NE(output.find("gs"), string::npos);
	EXPECT_NE(output.find("la"), string::npos);
}

TEST_F(HandlerTest, ShowShortcut) {
	db->get_shortcuts_table().add_shortcut("gs", "git status");
	auto [ret, output] = run_enter({"show", "gs"});
	EXPECT_EQ(ret, 0);
	EXPECT_NE(output.find("gs"), string::npos);
	EXPECT_NE(output.find("git status"), string::npos);
}

TEST_F(HandlerTest, ShowMissingShortcut) {
	auto [ret, output] = run_enter({"show", "missing"});
	EXPECT_EQ(ret, 1);
}

TEST_F(HandlerTest, ShowWrongArgCount) {
	auto [ret, output] = run_enter({"show"});
	EXPECT_EQ(ret, 1);
}

// ---- Tab completion ----

TEST_F(HandlerTest, TabCompletionPartial) {
	string mockfs = config->get_init_path();
	vector<string> args = {"dv-binary", "--tab", "dv", "1"};
	vector<const char*> argv_ptrs;
	for (const auto& a : args) argv_ptrs.push_back(a.c_str());

	testing::internal::CaptureStdout();
	testing::internal::CaptureStderr();
	int ret = handler->handle_tab(argv_ptrs.size(), const_cast<char**>(argv_ptrs.data()));
	testing::internal::GetCapturedStderr();
	string output = testing::internal::GetCapturedStdout();

	EXPECT_EQ(ret, 0);
	EXPECT_NE(output.find(mockfs + "/1"), string::npos);
}

TEST_F(HandlerTest, TabCompletionTooFewArgs) {
	const char* argv[] = {"dv-binary", "--tab", "dv"};
	testing::internal::CaptureStderr();
	int ret = handler->handle_tab(3, const_cast<char**>(argv));
	testing::internal::GetCapturedStderr();
	EXPECT_EQ(ret, 1);
}
