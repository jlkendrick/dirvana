#ifndef PATHS_TABLE_H
#define PATHS_TABLE_H

#include "Table.h"

class PathsTable : public Table {
public:
		PathsTable(Database& db) : Table(db) {}

		void create_table() const override;
		void drop_table() const override;
		std::vector<std::string> query(const std::string& input) const override;

		size_t count_existing_directories() const;
		void bulk_insert(const std::vector<std::tuple<std::string, std::string>>& rows);
		void delete_paths(const std::vector<std::string>& paths);
		bool should_exclude(const std::string& dir_name, const std::string& path) const;
		void select_all_paths(std::function<void(std::string)> callback) const;
};

#endif // PATHS_TABLE_H