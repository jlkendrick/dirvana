#include <gtest/gtest.h>

#include "DirectoryCompleter.h"

#include <iostream>
#include <string>

using namespace std;

TEST(DirectoryCompleter, Initialization) {
	string root = "/Users/jameskendrick/Code/Projects/dirvana/cpp/mockfs";
	DirectoryCompleter completer(root);

	EXPECT_EQ(completer.get_size(), 10);
}

void check(const string& root, const vector<string>& completions, const vector<string>& expected) {
	EXPECT_EQ(completions.size(), expected.size());
	for (int i = 0; i < completions.size(); i++) {
		EXPECT_EQ(completions[i], root + expected[i]);
	}
}

TEST(DirectoryCompleter, Completion) {
	string root = "/Users/jameskendrick/Code/Projects/dirvana/cpp/mockfs";
	DirectoryCompleter completer(root);

	auto completions = completer.complete("0");
	check(root, completions, {});

	completions = completer.complete("1");
	check(root, completions, {"/1", "/1/1", "/1/1/1"});

	completions = completer.complete("2");
	check(root, completions, {"/2", "/2/2"});

	completions = completer.complete("3");
	check(root, completions, {"/3"});

	completions = completer.complete("4");
	check(root, completions, {"/1/1/1/4", "/4", "/3/4", "/2/2/4"});

}

TEST(DirectoryCompleter, Access) {
	string root = "/Users/jameskendrick/Code/Projects/dirvana/cpp/mockfs";
	DirectoryCompleter completer(root);

	completer.access(root + "/0");
	check(root, completer.complete("0"), {"/0"});

	completer.access(root + "/1/1");
	check(root, completer.complete("1"), {"/1/1", "/1", "/1/1/1"});

	completer.access(root + "/1/1/1");
	check(root, completer.complete("1"), {"/1/1/1", "/1/1", "/1"});

	completer.access(root + "/1/1/1");
	check(root, completer.complete("1"), {"/1/1/1", "/1/1", "/1"});
}