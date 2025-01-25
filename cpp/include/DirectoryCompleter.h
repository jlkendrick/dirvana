#ifndef DIRECTORYCOMPLETER_H
#define DIRECTORYCOMPLETER_H

#include "RecentlyAccessedCache.h"

#include <string>
#include <vector>

class DirectoryCompleter {
public:
	// Constructor
	DirectoryCompleter(const std::string& rootdir);

	// Destructor
	~DirectoryCompleter() = default;

	// Getters
	int get_size() const { return directories.get_size(); }

private:
	std::string rootdir;
	RecentlyAccessedCache directories;

	// Constructor helper functions
	void collect_directories();
};

#endif // DIRECTORYCOMPLETER_H