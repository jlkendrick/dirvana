#include "Database.h"

Database::Database(const Config& config) : db(config.get_db_path()) {
	// Create the necessary table if it doesn't exist
	db << "CREATE TABLE IF NOT EXISTS paths ("
		  "id INTEGER PRIMARY KEY AUTOINCREMENT, "
		  "path TEXT NOT NULL, "
		  "dir_name TEXT NOT NULL, "
		  "last_accessed INTEGER NOT NULL, "
		  "access_count INTEGER NOT NULL DEFAULT 0"
		  ");";
	
}