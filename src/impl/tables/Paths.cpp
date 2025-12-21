#include "tables/Paths.h"
#include "Database.h"
#include "utils/Helpers.h"

#include <unordered_set>


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
	std::unordered_set<std::string> path_set; // To avoid duplicates
	
	try {
		// First, regardless of the matching type, check for exact matches
		std::string exact_query = std::format("SELECT path FROM paths WHERE dir_name {} {} ORDER BY {} DESC LIMIT {};",
			"=",
			get_query_pattern(dir_name, "exact"),
			db.get_config().get_promotion_strategy() == PromotionStrategy::RECENTLY_ACCESSED ? "last_accessed" : "access_count",
			db.get_config().get_max_results());
		// On the first insert, we can just insert directly without checking for duplicates
		db << exact_query >> [&](std::string path) {
			path_rankings.push_back(path);
			path_set.insert(path);
		};

		// If the matching type is not exact, we need to do a second query to rank the non-exact matches after the exact ones
		if (db.get_config().get_matching_type() != MatchingType::Exact) {
			std::string non_exact_query = std::format("SELECT path FROM paths WHERE dir_name {} {} ORDER BY {} DESC LIMIT {};",
				db.get_config().get_matching_type() == MatchingType::Exact ? "=" : "LIKE",
				get_query_pattern(dir_name),
				db.get_config().get_promotion_strategy() == PromotionStrategy::RECENTLY_ACCESSED ? "last_accessed" : "access_count",
				db.get_config().get_max_results());
			// For non-exact matches, we need to check for duplicates
			db << non_exact_query >> [&](std::string path) {
				if (path_set.find(path) == path_set.end()) {
					path_rankings.push_back(path);
					path_set.insert(path);
				}
			};
		}

	} catch (const sqlite::sqlite_exception& e) {
		std::cerr << "Error querying database: " << e.what() << std::endl;
	}
	
	return path_rankings;
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

bool PathsTable::should_exclude(const std::string& dir_name, const std::string& path) const {
	// Check if the directory should be excluded based on the exclusion rules
	std::vector<ExclusionRule> exclusion_rules = db.get_config().get_exclusion_rules();
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