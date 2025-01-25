#include <gtest/gtest.h>

#include "DirectoryCompleter.h"

#include <iostream>
#include <string>

using namespace std;

TEST(DirectoryCompleter, Initialization) {
	string root = "/Users/jameskendrick/Code/Projects/dirvana/cpp/mockfs";
	DirectoryCompleter completer(root);

	EXPECT_EQ(completer.get_size(), 8);
}

TEST(DirectoryCompleter, Completion) {
	string root = "/Users/jameskendrick/Code/Projects/dirvana/cpp/mockfs";
	DirectoryCompleter completer(root);

	auto completions = completer.complete("zzz");
	EXPECT_EQ(completions.size(), 0);

	completions = completer.complete("d");
	EXPECT_EQ(completions.size(), 1);

	completions = completer.complete("dir1");
	EXPECT_EQ(completions.size(), 1);

	completions = completer.complete("dir2");
	EXPECT_EQ(completions.size(), 2);

	completions = completer.complete("dir3");
	EXPECT_EQ(completions.size(), 3);
}