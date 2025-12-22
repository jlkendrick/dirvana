#include "tables/Shortcuts.h"
#include "Database.h"


void ShortcutsTable::create_table() const {
	try {
		db << "BEGIN TRANSACTION;";
		
		db << "CREATE TABLE IF NOT EXISTS shortcuts ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"shortcut TEXT NOT NULL, "
		"command TEXT NOT NULL, "
		"last_accessed INTEGER NOT NULL, "
		"access_count INTEGER NOT NULL DEFAULT 0"
		");";
		db << "CREATE UNIQUE INDEX IF NOT EXISTS idx_shortcut ON shortcuts (shortcut);";
		db << "CREATE INDEX IF NOT EXISTS idx_shortcuts_recency ON shortcuts (shortcut, last_accessed DESC);";
		db << "CREATE INDEX IF NOT EXISTS idx_shortcuts_freq ON shortcuts (shortcut, access_count DESC);";
		
		db << "COMMIT;";
	} catch (const sqlite::sqlite_exception& e) {
		db << "ROLLBACK;";
		std::cerr << "Error creating shortcuts table: " << e.what() << std::endl;
	}
}


void ShortcutsTable::drop_table() const {
	db << "DROP TABLE IF EXISTS shortcuts;";
}


std::vector<std::string> ShortcutsTable::query(const std::string& input) const {
	std::vector<std::string> results;
	
	// We only do exact matches for shortcuts (might change this in the future)
	std::string exact_query = std::format("SELECT command FROM shortcuts WHERE shortcut = '{}' LIMIT 1;", input);
	db << exact_query >> [&](std::string command) {
		results.push_back(command);
	};

	
	return results;
}


void ShortcutsTable::access(const std::string& command) {
	// This function is a no-op for shortcuts (for now)
	return;
}


void ShortcutsTable::add_shortcut(const std::string& shortcut, const std::string& command) {
	long long time_now = Time::now();

	try {
		db << "INSERT INTO shortcuts (shortcut, command, last_accessed) VALUES (?, ?, ?);"
			<< shortcut
			<< command
			<< time_now;
	} catch (const sqlite::sqlite_exception& e) {
		std::cerr << "Error adding shortcut to database: " << e.what() << std::endl;
	}
}