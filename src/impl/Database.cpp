#include "Database.h"

#include <unordered_set>


Database::Database(const Config& config) : db(config.get_db_path()), config(config), paths_table(*this), shortcuts_table(*this) {
	// Create the necessary table if it doesn't exist
	paths_table.create_table();
	shortcuts_table.create_table();
}


bool Database::build(const std::string& init_path) {
	// Get count of existing directories before dropping the table
	size_t old_dirs_count = paths_table.count_existing_directories();

	// Collect directories and insert them into the database
	auto rows = paths_table.collect_directories(init_path);
	
	// Check if we collected significantly fewer directories than expected
	// This could indicate a filesystem scanning failure
	// Only perform this check if we had an existing table with directories
	if (old_dirs_count > 0 && rows.size() < old_dirs_count / 10) {
		std::cerr << "Warning: Collected only " << rows.size() << " directories vs " << old_dirs_count 
		          << " in database. This might indicate a filesystem scanning issue. "
		          << "Skipping directory deletion to prevent data loss." << std::endl;
		return false;
		
	} else {
		// Drop the table and recreate it
		paths_table.drop_table();
		paths_table.create_table();
		paths_table.bulk_insert(rows);
		return true;
	}
	
	if (rows.empty()) {
		std::cerr << "No directories found to index from path: " << init_path << std::endl;
		return false;
	}

	paths_table.bulk_insert(rows);
	return true;
}


bool Database::refresh(const std::string& init_path) {
	// Fetch all existing directories and associated data from the database
	std::unordered_set<std::string> old_dirs;
	paths_table.select_all_paths([&old_dirs](std::string path) {
		old_dirs.insert(path);
	});
	std::vector<std::tuple<std::string, std::string>> new_rows;

	// Collect directories and perform a diff with the old directories
	auto rows = paths_table.collect_directories(init_path);
	if (rows.empty()) {
		std::cerr << "No directories found to index from path: " << init_path << std::endl;
		return false;
	}
	
	// Check if we collected significantly fewer directories than expected
	// This could indicate a filesystem scanning failure
	if (rows.size() < old_dirs.size() / 10) {
		std::cerr << "Warning: Collected only " << rows.size() << " directories vs " << old_dirs.size() 
		          << " in database. This might indicate a filesystem scanning issue. "
		          << "Skipping directory deletion to prevent data loss." << std::endl;
		
		// Only add new directories, don't delete existing ones
		for (const auto& [path, dir_name] : rows) {
			if (old_dirs.find(path) == old_dirs.end())
				new_rows.push_back({ path, dir_name });
		}
		paths_table.bulk_insert(new_rows);
		return true;
	}
	
	// Normal refresh logic when we have a reasonable number of directories
	for (const auto& [path, dir_name] : rows) {
		if (old_dirs.find(path) == old_dirs.end())
			new_rows.push_back({ path, dir_name });
		else
			old_dirs.erase(path);
	}

	// Insert new directories into the database and delete old ones
	paths_table.bulk_insert(new_rows);
	paths_table.delete_paths(std::vector<std::string>(old_dirs.begin(), old_dirs.end()));
	return true;
}