#include <gtest/gtest.h>

#include "caches/FrequencyBasedCache.h"
#include "utils/CacheEntry.h"

#include <string>
#include <vector>

using namespace std;

TEST(FrequencyBasedCache, Initialization) {
	FrequencyBasedCache cache;
	EXPECT_EQ(cache.get_size(), 0);
}

TEST(FrequencyBasedCache, AddingPaths) {
	FrequencyBasedCache cache;

	// Add a single path
	cache.add("/path/to/dir1");
	EXPECT_EQ(cache.get_size(), 1);
	EXPECT_TRUE(cache.contains("/path/to/dir1"));

	// Add multiple paths
	cache.add("/path/to/dir2");
	cache.add("/path/to/dir3");
	EXPECT_EQ(cache.get_size(), 3);
	EXPECT_TRUE(cache.contains("/path/to/dir2"));
	EXPECT_TRUE(cache.contains("/path/to/dir3"));

	// Verify initial order (during initialization, we insert at the back)
	vector<string> paths = cache.get_all_entries();
	EXPECT_EQ(paths.size(), 3);
	EXPECT_EQ(paths[0], "/path/to/dir1");
	EXPECT_EQ(paths[1], "/path/to/dir2");
	EXPECT_EQ(paths[2], "/path/to/dir3");
}

TEST(FrequencyBasedCache, FrequencyOrdering) {
	FrequencyBasedCache cache;

	// Add paths and access them different numbers of times
	cache.add("/path/to/dir3"); // count: 1
	cache.add("/path/to/dir2"); // count: 1
	cache.add("/path/to/dir1"); // count: 1

	cache.access("/path/to/dir1"); // count: 2
	cache.access("/path/to/dir1"); // count: 3

	cache.access("/path/to/dir2"); // count: 2

	// Verify frequency-based ordering
	vector<string> paths = cache.get_all_entries();
	EXPECT_EQ(paths.size(), 3);
	EXPECT_EQ(paths[0], "/path/to/dir1"); // Most frequent (3)
	EXPECT_EQ(paths[1], "/path/to/dir2"); // Second most frequent (2)
	EXPECT_EQ(paths[2], "/path/to/dir3"); // Least frequent (1)
}

TEST(FrequencyBasedCache, DuplicateHandling) {
	FrequencyBasedCache cache;

	// Add paths and verify size
	cache.add("/path/to/dir2");
	cache.add("/path/to/dir1");
	cache.add("/path/to/dir1"); // Should do nothing

	EXPECT_EQ(cache.get_size(), 2);
	vector<string> paths = cache.get_all_entries();
	EXPECT_EQ(paths[0], "/path/to/dir2"); // Original order
	EXPECT_EQ(paths[1], "/path/to/dir1"); // Original order
}