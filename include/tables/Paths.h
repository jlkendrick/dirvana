#ifndef PATHS_TABLE_H
#define PATHS_TABLE_H

#include "Table.h"

class PathsTable : public Table {
public:
		PathsTable(Database& db) : Table(db) {}

		void create_table() const override;
		void drop_table() const override;
};

#endif // PATHS_TABLE_H