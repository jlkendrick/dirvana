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