#ifndef DATABASE_H
#define DATABASE_H

#include "Config.h"

#include <sqlite_modern_cpp.h>


class Database {
public:

	Database(const Config& config);
	
	void build();
	void refresh();
	std::vector<std::string> query(const std::string& dir_name) const;
	void access(const std::string& path);

private:
	mutable sqlite::database db;
	const Config& config;

	void create_table() {
		db << "CREATE TABLE IF NOT EXISTS paths ("
			  "id INTEGER PRIMARY KEY AUTOINCREMENT, "
			  "path TEXT NOT NULL, "
			  "dir_name TEXT NOT NULL, "
			  "last_accessed INTEGER NOT NULL, "
			  "access_count INTEGER NOT NULL DEFAULT 0"
			  ");";
	};
	void drop_table() { db << "DROP TABLE IF EXISTS paths;"; };

	template <typename Callback>
	void select_all_paths(Callback callback) {
		db << "SELECT path FROM paths;"
			>> [&callback](std::string path) {
				callback(path);
			};
	};
	void bulk_insert(const std::vector<std::tuple<std::string, std::string, int>>& rows);
	void delete_paths(const std::vector<std::string>& paths);

	std::vector<std::tuple<std::string, std::string, int>> collect_directories();
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
};

#endif // DATABASE_H