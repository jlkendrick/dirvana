#include "DirectoryCompleter.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

DirectoryCompleter::DirectoryCompleter(const std::string& rootdir) : rootdir(rootdir) {
	collect_directories();
}

void DirectoryCompleter::collect_directories() {
	try {
		for (auto it = fs::recursive_directory_iterator(rootdir); it != fs::recursive_directory_iterator(); ++it) {
			const auto& entry = *it;

			// Skip hidden directories
			if (entry.path().filename().string()[0] == '.') {
				if (entry.is_directory())
					// Skip the entire directory
					it.disable_recursion_pending();
					
				continue;
			}

			if (fs::is_directory(entry))
				directories.add(entry.path().string());
		}
		
	} catch (const fs::filesystem_error& e) {
		std::cerr << e.what() << std::endl;
	}
}