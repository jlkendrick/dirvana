#include "tables/Paths.h"

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

