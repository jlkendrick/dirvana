#include "Database.h"
#include "Helpers.h"
#include "Types.h"

#include <filesystem>
#include <sqlite_modern_cpp.h>
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
	select_all_paths([&old_dirs](std::string path) {
		old_dirs.insert(path);
	});
	std::vector<std::tuple<std::string, std::string>> new_rows;
	for (const auto& exclusion_rule : config.get_exclusion_rules()) {
		std::cout << "Exclusion rule: " << exclusion_rule.pattern << std::endl;

	}

	// Collect directories and perform a diff with the old directories
	auto rows = collect_directories();
	for (const auto& [path, dir_name] : rows) {
		if (old_dirs.find(path) == old_dirs.end())
			new_rows.push_back({ path, dir_name });
		else
			old_dirs.erase(path);
	}

	// Insert new directories into the database and delete old ones
	bulk_insert(new_rows);
	delete_paths(std::vector<std::string>(old_dirs.begin(), old_dirs.end()));
}

std::vector<std::string> Database::query(const std::string& input) const {
	std::string dir_name = get_dir_name(input);
	std::vector<std::string> paths;
	
	try {
		std::string query = std::format("SELECT path FROM paths WHERE dir_name {} {} ORDER BY {} DESC LIMIT {};",
			config.get_matching_type() == MatchingType::Exact ? "=" : "LIKE",
			get_query_pattern(dir_name),
			config.get_promotion_strategy() == PromotionStrategy::RECENTLY_ACCESSED ? "last_accessed" : "access_count",
			config.get_max_results());

		db << query >> [&](std::string path) {
			paths.push_back(path);
		};

	} catch (const sqlite::sqlite_exception& e) {
		std::cerr << "Error querying database: " << e.what() << std::endl;
	}
	
	return paths;
}

void Database::access(const std::string& path) {
	long long last_accessed = get_current_time();
	try {
		
		db << "UPDATE paths SET last_accessed = ?, access_count = access_count + 1 WHERE path = ?;"
			<< last_accessed
			<< path;

	} catch (const sqlite::sqlite_exception& e) {
		std::cerr << "Error updating database: " << e.what() << std::endl;
	}
}


std::vector<std::tuple<std::string, std::string>> Database::collect_directories() {
	std::vector<std::tuple<std::string, std::string>> rows;
	try {
		std::filesystem::recursive_directory_iterator it(config.get_init_path(), std::filesystem::directory_options::skip_permission_denied);
		std::filesystem::recursive_directory_iterator end;
		
		while (it != end) {
			const auto& entry = *it;

			// Get the deepest directory name
			std::string dir_name = get_dir_name(entry.path().string());

			// If the entry is a directory and it's not in the exclude list, add it to the PathMap
			if (std::filesystem::is_directory(entry)) {
				if (not dir_name.empty() and not should_exclude(dir_name, entry.path().string()))
					rows.push_back({entry.path().string(), dir_name});
				
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

void Database::bulk_insert(const std::vector<std::tuple<std::string, std::string>>& rows) {
	long long last_accessed = get_current_time();

	try {
		db << "BEGIN TRANSACTION;";
		
		auto stmt = db << "INSERT INTO paths (path, dir_name, last_accessed) VALUES (?, ?, ?);";
		for (const auto& [path, dir_name] : rows) {
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
	if (paths.empty())
		return;

	try {
		db << "BEGIN TRANSACTION;";
		std::string sql = "DELETE FROM paths WHERE path IN (";
		for (size_t i = 0; i < paths.size(); ++i) {
			sql += "?";
			if (i < paths.size() - 1)
				sql += ", ";
		}
		sql += ");";
		
		auto stmt = db << sql;
		for (const auto& path : paths) {
			stmt << path;
			stmt++;
		}

		db << "COMMIT;";
	} catch (const sqlite::sqlite_exception& e) {
		db << "ROLLBACK;";
		std::cerr << "Error deleting data from database: " << e.what() << std::endl;
	}
}