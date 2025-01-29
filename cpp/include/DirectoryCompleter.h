#ifndef DIRECTORYCOMPLETER_H
#define DIRECTORYCOMPLETER_H

#include "RecentlyAccessedCache.h"
#include "PathMap.h"
#include "DLLTraverser.h"

#include <string>
#include <vector>

class DirectoryCompleter {
public:
	
	// Custom constructor which initializes the completer with all of the directories in the given root directory
	// by inserting them into the PathMap
	DirectoryCompleter(const std::string& rootdir);

	// Custom constructor which initializes the completer with all of the directories in the given root directory
	// and excludes the given directories from the PathMap.
	// Should be used primarily for testing or debugging purposes.
	DirectoryCompleter(const std::string& rootdir, const std::vector<std::string>& exclude);

	~DirectoryCompleter() = default;

	// Returns the doubly linked list that can be used to construct the traverser
	const DoublyLinkedList& get_list_for(const std::string& dir) const;

	bool has_matches(const std::string& dir) const { return directories.contains(dir); }

	// Finds the matching cache for the given directory name and returns the paths in that cache
	std::vector<std::string> get_all_matches(const std::string& dir) const;

	// Indicates that the given path has been accessed and it's position in the cache should be updated
	void access(const std::string& path) { directories.add(path); }

	// Returns the number of directories in the completer
	int get_size() const { return directories.size; }

private:
	std::string rootdir;
	PathMap directories;

	// Stores the names of the directories that we should skip when initializing the PathMap
	std::vector<std::string> exclude = { "node_modules", "bin", "obj", "build" };

	// Returns true if the given directory should be excluded from the PathMap
	bool should_exclude(const std::string& dir) const;

	// Helper function to get the deepest directory name of a given path
	std::string get_deepest_dir(const std::string& path) const;

	// Private function used by the constructor to collect all of the directories in the root directory
	void collect_directories();
};

#endif // DIRECTORYCOMPLETER_H