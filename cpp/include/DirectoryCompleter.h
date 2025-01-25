#ifndef DIRECTORYCOMPLETER_H
#define DIRECTORYCOMPLETER_H

#include "RecentlyAccessedCache.h"
#include "PathMap.h"

#include <string>
#include <vector>

class DirectoryCompleter {
public:
	
	// Custom constructor which initializes the completer with all of the directories in the given root directory
	// by inserting them into the PathMap
	DirectoryCompleter(const std::string& rootdir);
	~DirectoryCompleter() = default;

	// Finds the matching cache for the given directory name and returns the paths in that cache
	std::vector<std::string> complete(const std::string& dir) const;

	// Returns the number of directories in the completer
	int get_size() const { return directories.size; }

private:
	std::string rootdir;
	PathMap directories;

	// Private function used by the constructor to collect all of the directories in the root directory
	void collect_directories();
};

#endif // DIRECTORYCOMPLETER_H