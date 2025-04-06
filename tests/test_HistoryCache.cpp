#include <gtest/gtest.h>

#include "HistoryCache.h"

#include <string>
#include <vector>

using namespace std;

TEST(HistoryCache, Initialization) {
	HistoryCache cache;
	EXPECT_EQ(cache.get_size(), 0);
	EXPECT_EQ(cache.get_current_index(), 0);
}

// Should have same behavior as the recently accessed cache
TEST(HistoryCache, AddingPaths) {
	HistoryCache cache;

	// Add a single path
	cache.add(string("/path/to/dir1"));
	EXPECT_EQ(cache.get_size(), 1);
	EXPECT_TRUE(cache.contains("/path/to/dir1"));
	EXPECT_EQ(cache.get_current_index(), 1);

	// Add multiple paths
	cache.add(string("/path/to/dir2"));
	cache.add(string("/path/to/dir3"));
	EXPECT_EQ(cache.get_size(), 3);
	EXPECT_TRUE(cache.contains("/path/to/dir2"));
	EXPECT_TRUE(cache.contains("/path/to/dir3"));
	EXPECT_EQ(cache.get_current_index(), 3);

	// Verify order (most recent at the back)
	vector<string> paths = cache.get_all_paths();
	EXPECT_EQ(paths.size(), 3);
	EXPECT_EQ(paths[0], "/path/to/dir1");
	EXPECT_EQ(paths[1], "/path/to/dir2");
	EXPECT_EQ(paths[2], "/path/to/dir3");
}

// Should have same behavior as the recently accessed cache
TEST(HistoryCache, PromotingPaths) {
	HistoryCache cache;

	// Add paths in order
	cache.add(string("/path/to/dir1"));
	cache.add(string("/path/to/dir2"));
	cache.add(string("/path/to/dir3"));

	// Promote the first path
	cache.access(string("/path/to/dir1"));
	vector<string> paths = cache.get_all_paths();
	EXPECT_EQ(paths[0], "/path/to/dir1");
	EXPECT_EQ(paths[1], "/path/to/dir2");
	EXPECT_EQ(paths[2], "/path/to/dir3");

	// Promote a middle path
	cache.access(string("/path/to/dir2"));
	paths = cache.get_all_paths();
	EXPECT_EQ(paths[0], "/path/to/dir2");
	EXPECT_EQ(paths[1], "/path/to/dir1");
	EXPECT_EQ(paths[2], "/path/to/dir3");
}

TEST(HistoryCache, DuplicateHandling) {
	HistoryCache cache;

	// Add the same path multiple times
	cache.add(string("/path/to/dir2"));
	cache.add(string("/path/to/dir1"));
	cache.add(string("/path/to/dir1")); // Should do nothing

	EXPECT_EQ(cache.get_size(), 2);
	EXPECT_EQ(cache.get_current_index(), 2);
	vector<string> paths = cache.get_all_paths();
	EXPECT_EQ(paths[0], "/path/to/dir2");
	EXPECT_EQ(paths[1], "/path/to/dir1");
}