#include "Database.h"

#include <chrono>
#include <filesystem>
#include <unordered_set>

Database::Database(const Config& config) : db(config.get_db_path()), config(config) {
	// Create the necessary table if it doesn't exist
	create_table();
}

void Database::build() {
	// Drop the table and recreate it
	drop_table();
	create_table();

	// Collect directories and insert them into the database
	auto rows = collect_directories();
	bulk_insert(rows);
}

void Database::refresh() {
	// Fetch all existing directories and associated data from the database
	std::unordered_set<std::string> old_dirs;
	select_all([&old_dirs](std::string path) {
		old_dirs.insert(path);
	});
	std::vector<std::tuple<std::string, std::string, int>> new_rows;

	// Collect directories and perform a diff with the old directories
	auto rows = collect_directories();
	for (const auto& [path, dir_name, last_accessed] : rows) {
		if (old_dirs.find(path) == old_dirs.end())
			new_rows.push_back({path, dir_name, last_accessed});
		else
			old_dirs.erase(path);
	}

	// Insert new directories into the database and delete old ones
	bulk_insert(new_rows);
	delete_paths(std::vector<std::string>(old_dirs.begin(), old_dirs.end()));
}


std::vector<std::tuple<std::string, std::string, int>> Database::collect_directories() {
	auto now = std::chrono::system_clock::now();
	auto duration_since_epoch = now.time_since_epoch();
	auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(duration_since_epoch).count();
	std::vector<std::tuple<std::string, std::string, int>> rows;
	try {
		std::filesystem::recursive_directory_iterator it(config.get_init_path(), std::filesystem::directory_options::skip_permission_denied);
		std::filesystem::recursive_directory_iterator end;
		
		while (it != end) {
			const auto& entry = *it;

			// Get the deepest directory name
			std::pair<bool, std::string> res = get_deepest_dir(entry.path().string());

			// If the entry is a directory and it's not in the exclude list, add it to the PathMap
			if (std::filesystem::is_directory(entry)) {
				std::string dir_name = res.second;
				if (res.first and not should_exclude(dir_name, entry.path().string()))
					rows.push_back({entry.path().string(), dir_name, seconds_since_epoch});
				
				// Otherwise, don't add it to the PathMap and disable recursion into it's children
				else
					it.disable_recursion_pending();
			}

			++it;
		}
		
	} catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Error scanning filesystem: " << e.what() << std::endl;
	}

	return rows;
}

bool Database::should_exclude(const std::string& dirname, const std::string& path) const {
	// Check if the directory should be excluded based on the exclusion rules
	std::vector<ExclusionRule> exclusion_rules = config.get_exclusion_rules();
	for (const auto& rule : exclusion_rules) {
		switch (rule.type) {
		case ExclusionType::Exact:
			if (dirname == rule.pattern or path == rule.pattern)
				return true;
				break;

		case ExclusionType::Prefix:
			if (dirname.size() >= rule.pattern.size() and
				dirname.compare(0, rule.pattern.size(), rule.pattern) == 0)
				return true;
			break;
			
		case ExclusionType::Suffix:
			if (dirname.size() >= rule.pattern.size() and
				dirname.compare(dirname.size() - rule.pattern.size(), rule.pattern.size(), rule.pattern) == 0)
				return true;
			break;
        
		case ExclusionType::Contains:
			if (dirname.find(rule.pattern) != std::string::npos)
				return true;
			break;
				
		}
	}

	return false;
}

void Database::bulk_insert(const std::vector<std::tuple<std::string, std::string, int>>& rows) {
	try {
		db << "BEGIN TRANSACTION;";
		
		auto stmt = db << "INSERT INTO paths (path, dir_name, last_accessed) VALUES (?, ?, ?);";
		for (const auto& [path, dir_name, last_accessed] : rows) {
			stmt << path << dir_name << last_accessed;
			stmt++;
		}

		db << "COMMIT;";
	} catch (const sqlite::sqlite_exception& e) {
		db << "ROLLBACK;";
		std::cerr << "Error inserting data into database: " << e.what() << std::endl;
	}
}

void Database::delete_paths(const std::vector<std::string>& paths) {
	try {
		db << "BEGIN TRANSACTION;";
		
		std::string formatted_paths;
		for (const auto& path : paths) {
			if (formatted_paths.empty())
				formatted_paths = "'" + path + "'";
			else
				formatted_paths += ", '" + path + "'";
		}
		auto stmt = db << "DELETE FROM paths WHERE path IN (?);";
		stmt << formatted_paths;
		stmt++;

		db << "COMMIT;";
	} catch (const sqlite::sqlite_exception& e) {
		db << "ROLLBACK;";
		std::cerr << "Error deleting data from database: " << e.what() << std::endl;
	}
}