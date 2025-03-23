// #include <gtest/gtest.h>

// #include "DirectoryCompleter.h"

// #include <string>
// #include <vector>

// using PathMap = DirectoryCompleter::PathMap;

// class PathMapTest : public ::testing::Test {
// protected:
// 	PathMap map;

// 	void SetUp() override {
// 		// Initialize with some test paths
// 		map.add("/Users/test/projects/src");
// 		map.add("/Users/test/other/src");
// 		map.add("/Users/test/documents/include");
// 	}
// };

// TEST_F(PathMapTest, AddAndRetrievePaths) {
// 	// Test retrieving paths for 'src' directory
// 	auto src_paths = map.get_matches("src");
// 	ASSERT_EQ(src_paths.size(), 2);
// 	EXPECT_TRUE(std::find(src_paths.begin(), src_paths.end(), "/Users/test/projects/src") != src_paths.end());
// 	EXPECT_TRUE(std::find(src_paths.begin(), src_paths.end(), "/Users/test/other/src") != src_paths.end());

// 	// Test retrieving paths for 'include' directory
// 	auto include_paths = map.get_matches("include");
// 	ASSERT_EQ(include_paths.size(), 1);
// 	EXPECT_EQ(include_paths[0], "/Users/test/documents/include");
// }

// TEST_F(PathMapTest, NonExistentDirectory) {
// 	// Test retrieving paths for a directory that doesn't exist
// 	auto paths = map.get_matches("nonexistent");
// 	EXPECT_TRUE(paths.empty());
// }

// TEST_F(PathMapTest, AddInvalidPath) {
// 	// Test adding an invalid path (no directory separator)
// 	map.add("invalid_path");
// 	auto paths = map.get_matches("invalid_path");
// 	EXPECT_TRUE(paths.empty());
// }

// TEST_F(PathMapTest, SizeTracking) {
// 	PathMap new_map;
// 	EXPECT_EQ(new_map.get_size(), 0);

// 	new_map.add("/path/to/dir1");
// 	EXPECT_EQ(new_map.get_size(), 1);

// 	new_map.add("/another/path/to/dir1");
// 	EXPECT_EQ(new_map.get_size(), 2);

// 	// Invalid path should not increase size
// 	new_map.add("invalid_path");
// 	EXPECT_EQ(new_map.get_size(), 2);
// }

// TEST_F(PathMapTest, CacheOrdering) {
// 	PathMap order_map;
// 	order_map.add("/first/path/dir");
// 	order_map.add("/second/path/dir");

// 	auto paths = order_map.get_matches("dir");
// 	ASSERT_EQ(paths.size(), 2);
// 	// Most recently added path should be last during initialization
// 	EXPECT_EQ(paths[0], "/first/path/dir");
// 	EXPECT_EQ(paths[1], "/second/path/dir");
// }