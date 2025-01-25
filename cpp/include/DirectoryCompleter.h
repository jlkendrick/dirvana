#ifndef DIRECTORYCOMPLETER_H
#define DIRECTORYCOMPLETER_H

#include "RecentlyAccessedCache.h"
#include "PathMap.h"

#include <string>

class DirectoryCompleter {
public:
	// Constructor
	DirectoryCompleter(const std::string& rootdir);

	// Destructor
	~DirectoryCompleter() = default;

	// Getters
	int get_size() const { return directories.size; }

private:
	string rootdir;
	PathMap directories;

	// Private helper functions
	void collect_directories();
};

#endif // DIRECTORYCOMPLETER_H