#include "tables/Paths.h"
#include "Database.h"
#include "utils/Helpers.h"

#include <future>

void PathsTable::create_table() const {
	try {
		db << "BEGIN TRANSACTION;";
		
		db << "CREATE TABLE IF NOT EXISTS paths ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"path TEXT NOT NULL, "
		"dir_name TEXT NOT NULL, "
		"last_accessed INTEGER NOT NULL, "
		"access_count INTEGER NOT NULL DEFAULT 0"
		");";
		db << "CREATE UNIQUE INDEX IF NOT EXISTS idx_path ON paths (path);";
		db << "CREATE INDEX IF NOT EXISTS idx_paths_dir_recency ON paths (dir_name, last_accessed DESC);";
		db << "CREATE INDEX IF NOT EXISTS idx_paths_dir_freq ON paths (dir_name, access_count DESC);";
		
		db << "COMMIT;";
	} catch (const sqlite::sqlite_exception& e) {
		db << "ROLLBACK;";
		std::cerr << "Error creating paths table: " << e.what() << std::endl;
	}
}


void PathsTable::drop_table() const {
	db << "DROP TABLE IF EXISTS paths;";
}


std::vector<std::string> PathsTable::query(const std::string& input) const {
	std::string dir_name = get_dir_name(input);
	std::vector<std::string> path_rankings;
	const std::string sort_col = db.get_config().get_promotion_strategy() == PromotionStrategy::RECENTLY_ACCESSED
		? "last_accessed" : "access_count";
	const int max_results = db.get_config().get_max_results();

	try {
		if (db.get_config().get_matching_type() == MatchingType::Exact) {
			db << "SELECT path FROM paths WHERE dir_name = ? ORDER BY " + sort_col + " DESC LIMIT ?;"
			   << dir_name << max_results
			   >> [&](std::string path) { path_rankings.push_back(path); };
		} else {
			// Single query: exact matches sort first (rank 0), fuzzy matches second (rank 1).
			// Each path row appears at most once, so no dedup set is needed.
			std::string like_pattern = get_query_pattern(dir_name);
			db << "SELECT path FROM paths WHERE dir_name = ? OR dir_name LIKE ? "
			      "ORDER BY CASE WHEN dir_name = ? THEN 0 ELSE 1 END ASC, " + sort_col + " DESC LIMIT ?;"
			   << dir_name << like_pattern << dir_name << max_results
			   >> [&](std::string path) { path_rankings.push_back(path); };
		}
	} catch (const sqlite::sqlite_exception& e) {
		std::cerr << "Error querying database: " << e.what() << std::endl;
	}

	return path_rankings;
}


void PathsTable::access(const std::string& path) {
	long long time_now = Time::now();
	try {
		db << "UPDATE paths SET last_accessed = ?, access_count = access_count + 1 WHERE path = ?;"
			<< time_now
			<< path;
	} catch (const sqlite::sqlite_exception& e) {
		std::cerr << "Error updating database: " << e.what() << std::endl;
	}
}


std::vector<std::tuple<std::string, std::string>> PathsTable::collect_directories(const std::string& init_path) {
	const std::vector<ExclusionRule> exclusion_rules = db.get_config().get_exclusion_rules();

	// Scans the subtree rooted at `root`, returning all non-excluded directories within it.
	// Captures exclusion_rules by ref (const, read-only) and this for should_exclude (no shared state after our refactor).
	auto scan_subtree = [&](const std::string& root) {
		std::vector<std::tuple<std::string, std::string>> local_rows;
		try {
			std::filesystem::recursive_directory_iterator it(root, std::filesystem::directory_options::skip_permission_denied);
			std::filesystem::recursive_directory_iterator end;
			while (it != end) {
				const auto& entry = *it;
				std::string dir_name = get_dir_name(entry.path().string());
				if (std::filesystem::is_directory(entry)) {
					if (not dir_name.empty() and not should_exclude(dir_name, entry.path().string(), exclusion_rules))
						local_rows.push_back({entry.path().string(), dir_name});
					else
						it.disable_recursion_pending();
				}
				++it;
			}
		} catch (const std::filesystem::filesystem_error& e) {
			std::cerr << "Error scanning " << root << ": " << e.what() << std::endl;
		}
		return local_rows;
	};

	// One async task per non-excluded top-level subdirectory of init_path.
	std::vector<std::future<std::vector<std::tuple<std::string, std::string>>>> futures;
	std::vector<std::tuple<std::string, std::string>> rows;

	try {
		for (const auto& entry : std::filesystem::directory_iterator(init_path, std::filesystem::directory_options::skip_permission_denied)) {
			if (not std::filesystem::is_directory(entry))
				continue;
			std::string dir_name = get_dir_name(entry.path().string());
			if (dir_name.empty() or should_exclude(dir_name, entry.path().string(), exclusion_rules))
				continue;
			rows.push_back({entry.path().string(), dir_name});
			futures.push_back(std::async(std::launch::async, scan_subtree, entry.path().string()));
		}
	} catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Error scanning " << init_path << ": " << e.what() << std::endl;
	}

	for (auto& fut : futures) {
		auto subtree = fut.get();
		rows.insert(rows.end(), std::make_move_iterator(subtree.begin()), std::make_move_iterator(subtree.end()));
	}

	return rows;
}


// Collect all files that are direct children of the init path
std::vector<std::string> PathsTable::collect_files(const std::string& init_path) const {
	std::vector<std::string> files;
	std::filesystem::directory_iterator it(init_path);
	for (const auto& entry : it) {
		if (std::filesystem::is_regular_file(entry))
			files.push_back(entry.path().string());
	}
	return files;
}

size_t PathsTable::count_existing_directories() const {
	try {
		size_t count = 0;
		db << "SELECT COUNT(*) FROM paths;" >> count;
		return count;
	} catch (const sqlite::sqlite_exception& e) {
		// If table doesn't exist or any error occurs, return 0
		return 0;
	}
};


void PathsTable::bulk_insert(const std::vector<std::tuple<std::string, std::string>>& rows) {
	if (rows.empty())
		return;

	long long last_accessed = Time::now();

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


void PathsTable::delete_paths(const std::vector<std::string>& paths) {
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

bool PathsTable::should_exclude(const std::string& dir_name, const std::string& path, const std::vector<ExclusionRule>& exclusion_rules) const {
	for (const auto& rule : exclusion_rules) {
		switch (rule.type) {
		case ExclusionType::Exact:
			if (dir_name == rule.pattern or path == rule.pattern)
				return true;
			break;

		case ExclusionType::Prefix:
			if (dir_name.size() >= rule.pattern.size() and
				dir_name.compare(0, rule.pattern.size(), rule.pattern) == 0)
				return true;
			break;
			
		case ExclusionType::Suffix:
			if (dir_name.size() >= rule.pattern.size() and
				dir_name.compare(dir_name.size() - rule.pattern.size(), rule.pattern.size(), rule.pattern) == 0)
				return true;
			break;
        
		case ExclusionType::Contains:
			if (dir_name.find(rule.pattern) != std::string::npos)
				return true;
			break;
		
		default:
			std::cerr << "Unknown exclusion type: " << static_cast<int>(rule.type) << std::endl;
			return false;
		}
	}

	return false;
}


void PathsTable::select_all_paths(std::function<void(std::string)> callback) const {
	try {
		db << "SELECT path FROM paths;" >> [callback](std::string path) {
			callback(path);
		};
	} catch (const sqlite::sqlite_exception& e) {
		std::cerr << "Error selecting all paths: " << e.what() << std::endl;
	}
};