#ifndef DATABASE_H
#define DATABASE_H

#include "Config.h"

#include <sqlite_modern_cpp.h>


class Database {
public:

	Database(const Config& config);
	
	void build();
	void refresh();
	std::vector<std::string> query(const std::string& input) const;
	void access(const std::string& path);

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
		db << "SELECT path FROM paths;"
			>> [&callback](std::string path) {
				callback(path);
			};
	};
	void bulk_insert(const std::vector<std::tuple<std::string, std::string>>& rows);
	void delete_paths(const std::vector<std::string>& paths);

	std::vector<std::tuple<std::string, std::string>> collect_directories();
	bool should_exclude(const std::string& dirname, const std::string& path) const;
	std::string get_query_pattern(const std::string& dir_name) const {
		switch (config.get_matching_type()) {
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
	}
};

#endif // DATABASE_H