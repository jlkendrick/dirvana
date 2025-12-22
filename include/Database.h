#ifndef DATABASE_H
#define DATABASE_H

#include "Config.h"
#include "tables/Paths.h"
#include "tables/Shortcuts.h"

#include <sqlite_modern_cpp.h>


class Database {
public:

	Database(const Config& config);
	
	bool build(const std::string& init_path);
	bool refresh(const std::string& init_path);

	const Config& get_config() const { return config; }
	PathsTable& get_paths_table() { return paths_table; }
	ShortcutsTable& get_shortcuts_table() { return shortcuts_table; }

	auto operator<<(const std::string& sql) { return db << sql; }

private:
	mutable sqlite::database db;
	
	const Config& config;
	PathsTable paths_table;
	ShortcutsTable shortcuts_table;
};

#endif // DATABASE_H