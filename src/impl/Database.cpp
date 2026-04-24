#include "Database.h"


Database::Database(const Config& config) : db(config.get_db_path()), config(config), paths_table(*this), shortcuts_table(*this) {
	// Create the necessary table if it doesn't exist
	paths_table.create_table();
	shortcuts_table.create_table();
}


bool Database::build(const std::string& init_path, bool force) {
	// Get count of existing directories before dropping the table
	size_t old_dirs_count = paths_table.count_existing_directories();

	// Collect directories and insert them into the database
	auto rows = paths_table.collect_directories(init_path);
	
	if (rows.empty()) {
		std::cerr << "No directories found to index from path: " << init_path << std::endl;
		return false;
	}

	// Check if we collected significantly fewer directories than expected
	// This could indicate a filesystem scanning failure
	// Only perform this check if we had an existing table with directories
	if (old_dirs_count > 0 && rows.size() < old_dirs_count / 10 && !force) {
		std::cerr << "Warning: Collected only " << rows.size() << " directories vs " << old_dirs_count 
		          << " in database. This might indicate a filesystem scanning issue. "
		          << "Skipping directory deletion to prevent data loss." << std::endl;
		return false;
		
	}

	// Drop the table and recreate it
	paths_table.drop_table();
	paths_table.create_table();
	paths_table.bulk_insert(rows);

	return true;
}


bool Database::refresh(const std::string& init_path) {
	// Collect directories and perform a diff with the old directories
	auto rows = paths_table.collect_directories(init_path);
	if (rows.empty()) {
		std::cerr << "No directories found to index from path: " << init_path << std::endl;
		return false;
	}
	
	// Check if we collected significantly fewer directories than expected
	// This could indicate a filesystem scanning failure
	bool should_delete = true;
	size_t old_dirs_count = paths_table.count_existing_directories();
	if (rows.size() < old_dirs_count / 10) {
		std::cerr << "Warning: Collected only " << rows.size() << " directories vs " << old_dirs_count 
		          << " in database. This might indicate a filesystem scanning issue. "
		          << "Skipping directory deletion to prevent data loss." << std::endl;
		
		// Only add new directories, don't delete existing ones
		should_delete = false;
	}

	long long last_accessed = Time::now();

	try {
		db << "BEGIN TRANSACTION;";
		db << "DROP TABLE IF EXISTS temp_paths;";
		db << "CREATE TEMP TABLE temp_paths (path TEXT NOT NULL, dir_name TEXT NOT NULL);";
		auto stmt = db << "INSERT INTO temp_paths (path, dir_name) VALUES (?, ?);";
		for (const auto& [path, dir_name] : rows) {
			stmt << path << dir_name;
			stmt++;
		}
		db << "INSERT OR IGNORE INTO paths (path, dir_name, last_accessed) SELECT path, dir_name, ? FROM temp_paths;"
			 << last_accessed;
		if (should_delete)
			db << "DELETE FROM paths WHERE path NOT IN (SELECT path FROM temp_paths);";
		db << "DROP TABLE temp_paths;";
		db << "COMMIT;";
	} catch (const sqlite::sqlite_exception& e) {
		db << "ROLLBACK;";
		std::cerr << "Error refreshing database: " << e.what() << std::endl;
		return false;
	}

	return true;
}