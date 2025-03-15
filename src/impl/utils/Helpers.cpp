#include "Helpers.h"

#include <string>

// Helper function to return the deepest directory name in a path
// Ex. get_deepest_dir("/Users/jameskendrick/Code/Projects/dirvana/cpp/src") will return "src"
// We return a pair of bool and string to ensure that the path is valid
std::pair<bool, std::string> get_deepest_dir(const std::string& path) {
	size_t pos = path.find_last_of('/');
	if (pos >= std::string::npos)
		return std::make_pair(false, "");

	return std::make_pair(true, path.substr(pos + 1));
}