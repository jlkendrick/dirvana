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
		fs::recursive_directory_iterator it(rootdir, fs::directory_options::skip_permission_denied);
		fs::recursive_directory_iterator end;
		
		while (it != end) {
			const auto& entry = *it;

			// Get the deepest directory name
			std::string dir_name = get_deepest_dir(entry.path().string());

			// If the entry is a directory and it's not in the exclude list, add it to the PathMap
			if (fs::is_directory(entry)) {
				if (dir_name != "" && !should_exclude(dir_name)) {
					directories.add(entry.path().string(), dir_name);
				
				// Otherwise, don't add it to the PathMap and disable recursion into it's children
				} else {
					it.disable_recursion_pending();
				}
			}

			++it;
		}
		
	} catch (const fs::filesystem_error& e) {
		std::cerr << e.what() << std::endl;
	}
}

const std::shared_ptr<DoublyLinkedList> DirectoryCompleter::get_list_for(const std::string& dir) const {
	return directories.get_list_for(dir);
}

std::vector<std::string> DirectoryCompleter::get_all_matches(const std::string& dir) const {
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