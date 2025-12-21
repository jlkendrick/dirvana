#include "tables/Shortcuts.h"

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

