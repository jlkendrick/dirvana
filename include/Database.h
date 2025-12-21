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
	void access(const std::string& path);

	void add_shortcut(const std::string& shortcut, const std::string& command);
	
	const Config& get_config() const { return config; }

	auto operator<<(const std::string& sql) { return db << sql; }

private:
	mutable sqlite::database db;
	const Config& config;

	PathsTable paths_table;
	ShortcutsTable shortcuts_table;

	std::vector<std::tuple<std::string, std::string>> collect_directories(const std::string& init_path);
};

#endif // DATABASE_H