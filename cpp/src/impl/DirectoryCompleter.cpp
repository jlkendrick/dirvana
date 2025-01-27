#include "DirectoryCompleter.h"

#include "PathMap.h"

#include <memory>
#include <iostream>
#include <filesystem>


namespace fs = std::filesystem;

DirectoryCompleter::DirectoryCompleter(const std::string& rootdir) : rootdir(rootdir) {
	collect_directories();
}

DirectoryCompleter::DirectoryCompleter(const std::string& rootdir, const std::vector<std::string>& exclude) : rootdir(rootdir), exclude(exclude) {
	collect_directories();
}

void DirectoryCompleter::collect_directories() {
	try {
		for (auto it = fs::recursive_directory_iterator(rootdir); it != fs::recursive_directory_iterator(); ++it) {
			const auto& entry = *it;

			// Get the deepest directory name
			std::string dir_name = get_deepest_dir(entry.path().string());

			// If the entry is not a directory or the directory name is empty or should be excluded, skip it and it's subdirectories
			if (!fs::is_directory(entry) || dir_name == "" || should_exclude(dir_name)) {
				it.disable_recursion_pending();
				continue;
			}

			directories.add(entry.path().string(), dir_name);
		}
		
	} catch (const fs::filesystem_error& e) {
		std::cerr << e.what() << std::endl;
	}
}

std::vector<std::string> DirectoryCompleter::complete(const std::string& dir) const {
	return directories.get_all_paths(dir);
}

bool DirectoryCompleter::should_exclude(const std::string& dirname) const {
	// Exclude if the path is a dot directory
	if (dirname[0] == '.')
		return true;
	
	// Exclude if the path is in the exclude list
	for (const auto& exclude_dirname : exclude) {
		if (dirname == exclude_dirname)
			return true;
	}
	
	return false;
}

std::string DirectoryCompleter::get_deepest_dir(const std::string& path) const {
	// Return the deepest directory name or "" if it is invalid
	size_t pos = path.find_last_of('/');
	if (pos >= std::string::npos)
		return "";

	std::string dir = path.substr(pos + 1);
	return dir;
}