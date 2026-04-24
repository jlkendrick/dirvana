#include <gtest/gtest.h>

#include "utils/Helpers.h"

using namespace std;

// ---- get_dir_name ----

TEST(GetDirName, Nested) {
	EXPECT_EQ(get_dir_name("/a/b/c"), "c");
}

TEST(GetDirName, NoSlash) {
	EXPECT_EQ(get_dir_name("foo"), "foo");
}

TEST(GetDirName, TrailingSlash) {
	// Documents current behavior: trailing slash yields empty string
	EXPECT_EQ(get_dir_name("/a/b/"), "");
}

// ---- ArgParsing::process_args ----

// Helper: converts a vector of string tokens into the argc/argv form process_args expects
static tuple<bool, vector<string>, vector<Flag>> parse(vector<string> tokens) {
	vector<const char*> ptrs;
	for (const auto& t : tokens) ptrs.push_back(t.c_str());
	return ArgParsing::process_args(static_cast<int>(ptrs.size()), const_cast<char**>(ptrs.data()));
}

TEST(ProcessArgs, BasicPath) {
	auto [ok, cmds, flags] = parse({"dv-binary", "--enter", "dv", "mydir"});
	EXPECT_TRUE(ok);
	EXPECT_EQ(cmds, (vector<string>{"mydir"}));
	EXPECT_TRUE(flags.empty());
}

TEST(ProcessArgs, MultipleCommands) {
	auto [ok, cmds, flags] = parse({"dv-binary", "--enter", "dv", "add", "gs", "git status"});
	EXPECT_TRUE(ok);
	EXPECT_EQ(cmds, (vector<string>{"add", "gs", "git status"}));
	EXPECT_TRUE(flags.empty());
}

// Command must precede its flag so process_args can associate them correctly
TEST(ProcessArgs, FlagAlias) {
	auto [ok, cmds, flags] = parse({"dv-binary", "--enter", "dv", "build", "-r", "/some/path"});
	EXPECT_TRUE(ok);
	EXPECT_EQ(cmds, (vector<string>{"build"}));
	ASSERT_EQ(flags.size(), 1u);
	EXPECT_EQ(flags[0].flag, "root");
	EXPECT_EQ(flags[0].value, "/some/path");
}

TEST(ProcessArgs, FullFlagName) {
	auto [ok, cmds, flags] = parse({"dv-binary", "--enter", "dv", "build", "--root", "/some/path"});
	EXPECT_TRUE(ok);
	ASSERT_EQ(flags.size(), 1u);
	EXPECT_EQ(flags[0].flag, "root");
	EXPECT_EQ(flags[0].value, "/some/path");
}

TEST(ProcessArgs, BypassFlag) {
	auto [ok, cmds, flags] = parse({"dv-binary", "--enter", "dv", "--", "mydir"});
	EXPECT_TRUE(ok);
	EXPECT_TRUE(cmds.empty());
	ASSERT_EQ(flags.size(), 1u);
	EXPECT_EQ(flags[0].flag, "[bypass]");
	EXPECT_EQ(flags[0].value, "mydir");
}

TEST(ProcessArgs, SystemCommandBypass) {
	// "git" is a known system command — all args are passed through as-is
	auto [ok, cmds, flags] = parse({"dv-binary", "--enter", "dv", "git", "status"});
	EXPECT_TRUE(ok);
	EXPECT_EQ(cmds, (vector<string>{"git", "status"}));
	EXPECT_TRUE(flags.empty());
}

// ---- ArgParsing::has_flag / get_flag_value ----

TEST(ArgParsingFlags, HasFlagTrue) {
	vector<Flag> flags = {{"", "root", "/p"}};
	EXPECT_TRUE(ArgParsing::has_flag(flags, "root"));
}

TEST(ArgParsingFlags, HasFlagFalse) {
	vector<Flag> flags = {};
	EXPECT_FALSE(ArgParsing::has_flag(flags, "root"));
}

TEST(ArgParsingFlags, GetFlagValuePresent) {
	vector<Flag> flags = {{"", "root", "/p"}};
	EXPECT_EQ(ArgParsing::get_flag_value(flags, "root"), "/p");
}

TEST(ArgParsingFlags, GetFlagValueDefault) {
	vector<Flag> flags = {};
	EXPECT_EQ(ArgParsing::get_flag_value(flags, "root", "/default"), "/default");
}
