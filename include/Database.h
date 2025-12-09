#ifndef DATABASE_H
#define DATABASE_H

#include "Config.h"

#include <sqlite_modern_cpp.h>


class Database {
public:

	Database(const Config& config);
	
	bool build(const std::string& init_path);
	bool refresh(const std::string& init_path);
	std::vector<std::string> query(const std::string& input) const;
	void access(const std::string& path);

	void add_shortcut(const std::string& shortcut, const std::string& command);
	
	const Config& get_config() const { return config; }

private:
	mutable sqlite::database db;
	const Config& config;

	void create_table() {
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
			std::cerr << "Error creating table: " << e.what() << std::endl;
		}
	};
	void drop_table() { db << "DROP TABLE IF EXISTS paths;"; };

	template <typename Callback>
	void select_all_paths(Callback callback) {
		try {
			db << "SELECT path FROM paths;" >> callback;
		} catch (const sqlite::sqlite_exception& e) {
			std::cerr << "Error selecting all paths: " << e.what() << std::endl;
		}
	};
	
	size_t count_existing_directories() {
		try {
			size_t count = 0;
			db << "SELECT COUNT(*) FROM paths;" >> count;
			return count;
		} catch (const sqlite::sqlite_exception& e) {
			// If table doesn't exist or any error occurs, return 0
			return 0;
		}
	};
	void bulk_insert(const std::vector<std::tuple<std::string, std::string>>& rows);
	void delete_paths(const std::vector<std::string>& paths);

	std::vector<std::tuple<std::string, std::string>> collect_directories(const std::string& init_path);
	bool should_exclude(const std::string& dirname, const std::string& path) const;
	std::string get_query_pattern(const std::string& dir_name, const std::string& matching_type_override = "") const {
		switch (matching_type_override.empty() ? config.get_matching_type() : TypeConversions::s_to_matching_type(matching_type_override)) {
			case MatchingType::Exact:
				return "'" + dir_name + "'";
			case MatchingType::Prefix:
				return "'%" + dir_name + "%'";
			case MatchingType::Suffix:
				return "'%" + dir_name + "'";
			case MatchingType::Contains:
				return "'%" + dir_name + "%'";
		}
		return "";
	};
	long long get_current_time() const {
		auto now = std::chrono::system_clock::now();
		auto duration_since_epoch = now.time_since_epoch();
		auto micros_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(duration_since_epoch);

		return micros_since_epoch.count();
	};
};

#endif // DATABASE_H