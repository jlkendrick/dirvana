#ifndef HELPERS_H
#define HELPERS_H

#include <string>

std::pair<bool, std::string> get_deepest_dir(const std::string& path);
bool is_valid_path(const std::string& path);

#endif // HELPERS_H