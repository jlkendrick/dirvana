#ifndef TABLE_H
#define TABLE_H

#include <sqlite_modern_cpp.h>

#include "Database.h"


class Table {
public:
	Table(Database& db) : db(db) {}

	virtual void create_table() const = 0;
	virtual void drop_table() const = 0;

	Database& db;
};

#endif // TABLE_H