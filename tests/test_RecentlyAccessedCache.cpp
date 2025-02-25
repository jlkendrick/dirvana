#include <gtest/gtest.h>

#include "caches/RecentlyAccessedCache.h"

#include <string>
#include <vector>

using namespace std;

// ------------------------------------------- SELF-IMPLEMENTED RECENTLY ACCESSED CACHE -------------------------------------------
TEST(RecentlyAccessedCache, Initialization) {
	RecentlyAccessedCacheV1 cache;
	EXPECT_EQ(cache.get_size(), 0);
}

TEST(RecentlyAccessedCacheV1, AddingPaths) {
	RecentlyAccessedCacheV1 cache;

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

	// Verify order (most recent at the back)
	vector<string> paths = cache.get_all_paths();
	EXPECT_EQ(paths.size(), 3);
	EXPECT_EQ(paths[0], "/path/to/dir1");
	EXPECT_EQ(paths[1], "/path/to/dir2");
	EXPECT_EQ(paths[2], "/path/to/dir3");
}

TEST(RecentlyAccessedCacheV1, PromotingPaths) {
	RecentlyAccessedCacheV1 cache;

	// Add paths in order
	cache.add("/path/to/dir1");
	cache.add("/path/to/dir2");
	cache.add("/path/to/dir3");

	// Promote the first path
	cache.promote("/path/to/dir1");
	vector<string> paths = cache.get_all_paths();
	EXPECT_EQ(paths[0], "/path/to/dir1");
	EXPECT_EQ(paths[1], "/path/to/dir2");
	EXPECT_EQ(paths[2], "/path/to/dir3");

	// Promote a middle path
	cache.promote("/path/to/dir2");
	paths = cache.get_all_paths();
	EXPECT_EQ(paths[0], "/path/to/dir2");
	EXPECT_EQ(paths[1], "/path/to/dir1");
	EXPECT_EQ(paths[2], "/path/to/dir3");
}

TEST(RecentlyAccessedCacheV1, DuplicateHandling) {
	RecentlyAccessedCacheV1 cache;

	// Add the same path multiple times
	cache.add("/path/to/dir2");
	cache.add("/path/to/dir1");
	cache.add("/path/to/dir1"); // Should do nothing

	EXPECT_EQ(cache.get_size(), 2);
	vector<string> paths = cache.get_all_paths();
	EXPECT_EQ(paths[0], "/path/to/dir2");
	EXPECT_EQ(paths[1], "/path/to/dir1");
}

// ------------------------------------------- STD LIBRARY RECENTLY ACCESSED CACHE -------------------------------------------
TEST(RecentlyAccessedCacheV2, Initialization) {
	RecentlyAccessedCacheV2 cache;
	EXPECT_EQ(cache.get_size(), 0);
}

TEST(RecentlyAccessedCacheV2, AddingPaths) {
	RecentlyAccessedCacheV2 cache;

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

	// Verify order (most recent at the back)
	vector<string> paths = cache.get_all_paths();
	EXPECT_EQ(paths.size(), 3);
	EXPECT_EQ(paths[0], "/path/to/dir1");
	EXPECT_EQ(paths[1], "/path/to/dir2");
	EXPECT_EQ(paths[2], "/path/to/dir3");
}

TEST(RecentlyAccessedCacheV2, PromotingPaths) {
	RecentlyAccessedCacheV2 cache;

	// Add paths in order
	cache.add("/path/to/dir1");
	cache.add("/path/to/dir2");
	cache.add("/path/to/dir3");

	// Promote the first path
	cache.promote("/path/to/dir1");
	vector<string> paths = cache.get_all_paths();
	EXPECT_EQ(paths[0], "/path/to/dir1");
	EXPECT_EQ(paths[1], "/path/to/dir2");
	EXPECT_EQ(paths[2], "/path/to/dir3");

	// Promote a middle path
	cache.promote("/path/to/dir2");
	paths = cache.get_all_paths();
	EXPECT_EQ(paths[0], "/path/to/dir2");
	EXPECT_EQ(paths[1], "/path/to/dir1");
	EXPECT_EQ(paths[2], "/path/to/dir3");
}

TEST(RecentlyAccessedCacheV2, DuplicateHandling) {
	RecentlyAccessedCacheV2 cache;

	// Add the same path multiple times
	cache.add("/path/to/dir2");
	cache.add("/path/to/dir1");
	cache.add("/path/to/dir1"); // Should do nothing

	EXPECT_EQ(cache.get_size(), 2);
	vector<string> paths = cache.get_all_paths();
	EXPECT_EQ(paths[0], "/path/to/dir2");
	EXPECT_EQ(paths[1], "/path/to/dir1");
}